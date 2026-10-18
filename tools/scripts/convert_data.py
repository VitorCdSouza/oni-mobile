"""Convert the game's Unity data files for the Android player.

For every serialized file (the main data files and the AssetBundles in
StreamingAssets) this:

- recompresses DXT1/DXT5 textures to ASTC (Adreno GPUs cannot sample BC);
- optionally rewrites the OpenGLCore shaders as GLES 3 (see shader_gles.py);
- optionally rewrites the target platform in the file header.

All texture data is rewritten into a fresh .resS (next to a main file, or as
the bundle's own .resS entry). Everything else is copied or linked unchanged.

Usage:
  tools/venv/bin/python -I tools/scripts/convert_data.py <Game_Data> <out dir>
      [--block 6x6] [--platform android|keep] [--shaders gles|keep] [--only NAME ...] [--jobs N]
"""

import argparse
import multiprocessing
import os
import sys
import time

import astc_encoder
import texture2ddecoder
import UnityPy
from UnityPy.enums import BuildTarget
from UnityPy.files import BundleFile, SerializedFile
from UnityPy.streams import EndianBinaryWriter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import shader_gles  # noqa: E402

# UnityEngine.TextureFormat
DXT1, DXT5 = 10, 12
ASTC_FORMATS = {"4x4": 48, "5x5": 49, "6x6": 50, "8x8": 51, "10x10": 52, "12x12": 53}
# UnityEditor.BuildTarget
PLATFORMS = {"android": 13, "linux": 24}
# UnityEngine.Rendering.GraphicsDeviceType
GLES3 = 11
# The player reads these from the PlayerSettings in globalgamemanagers, which the
# PC build left at desktop values. Taken from an Android build of unity/player.
# With playerMinOpenGLESVersion 0 the player refuses to create a GLES device, and
# applicationEntry must match the GameActivity the APK is built with.
ANDROID_PLAYER_SETTINGS = {
    "playerMinOpenGLESVersion": 2,
    "androidApplicationEntry": 2,
    "mobileMTRenderingBaked": True,
    "androidUseSwappy": True,
    "androidRenderOutsideSafeArea": True,
    "androidResizeableActivity": True,
    "androidMaxAspectRatio": 2.4,
    # UIOrientation.AutoRotation, landscape only.
    "defaultScreenOrientation": 4,
    "allowedAutorotateToPortrait": False,
    "allowedAutorotateToPortraitUpsideDown": False,
    "allowedAutorotateToLandscapeRight": True,
    "allowedAutorotateToLandscapeLeft": True,
    # FrameTimingManager (CPU main and render thread, GPU time per frame), read by
    # OniMobile.FrameStats. A release player only records them with this set.
    "enableFrameTimingStats": True,
}
MAIN_FILES = ("globalgamemanagers", "globalgamemanagers.assets", "level0", "level1", "level2",
              "resources.assets", "sharedassets0.assets", "sharedassets1.assets", "sharedassets2.assets",
              "Resources/unity_builtin_extra")

_context = None
_block = None


def _init_worker(block):
    global _context, _block
    _block = block
    bx, by = block
    config = astc_encoder.ASTCConfig(astc_encoder.ASTCProfile.LDR, bx, by, 1,
                                     astc_encoder.ASTCQualityPreset.FAST)
    _context = astc_encoder.ASTCContext(config)


def _dxt_mips(fmt, width, height, mips):
    """Yields (offset, size, w, h) for each mip level of a DXT texture."""
    block_bytes = 8 if fmt == DXT1 else 16
    offset = 0
    for _ in range(mips):
        size = max(1, (width + 3) // 4) * max(1, (height + 3) // 4) * block_bytes
        yield offset, size, width, height
        offset += size
        width, height = max(1, width // 2), max(1, height // 2)


def encode_texture(job):
    """Worker: DXT bytes (all mips, all images) to ASTC bytes."""
    fmt, width, height, mips, images, data = job
    decode = texture2ddecoder.decode_bc1 if fmt == DXT1 else texture2ddecoder.decode_bc3
    # The decoders give BGRA; tell the encoder where each channel sits.
    swizzle = astc_encoder.ASTCSwizzle.from_str("bgra")
    out = []
    image_size = sum(size for _, size, _, _ in _dxt_mips(fmt, width, height, mips))
    for i in range(images):
        base = i * image_size
        for offset, size, w, h in _dxt_mips(fmt, width, height, mips):
            raw = data[base + offset:base + offset + size]
            bgra = decode(raw, w, h)
            image = astc_encoder.ASTCImage(astc_encoder.ASTCType.U8, w, h, 1, bgra)
            out.append(_context.compress(image, swizzle))
    return b"".join(out)


def texture_bytes(env, tree, data_dir):
    stream = tree.get("m_StreamData") or {}
    if not stream.get("path"):
        return bytes(tree["image data"])
    name = os.path.basename(stream["path"])
    if stream["path"].startswith("archive:"):
        # Bundle: the .resS is another entry of the same archive.
        return bytes(env.file.files[name].bytes[stream["offset"]:stream["offset"] + stream["size"]])
    with open(os.path.join(data_dir, name), "rb") as f:
        f.seek(stream["offset"])
        return f.read(stream["size"])


def _encode_or_keep(job):
    return encode_texture(job) if job[0] in (DXT1, DXT5) else job[5]


def convert_file(src, dst, data_dir, args, pool):
    """Rewrites one serialized file or bundle. Every texture's data goes into a new
    .resS with the original name (DXT ones recompressed, the rest copied), so the
    original .resS, full of dead DXT data, is not needed any more."""
    started = time.time()
    env = UnityPy.load(src)
    is_bundle = env.file.__class__.__name__ == "BundleFile"
    fmt_id = ASTC_FORMATS[args.block]

    jobs, textures = [], []
    shaders = 0
    res_entry = None
    for obj in env.objects:
        if obj.type.name == "Shader" and args.shaders == "gles":
            tree = obj.read_typetree()
            if shader_gles.convert_shader(tree) is not None:
                obj.save_typetree(tree)
                shaders += 1
            continue
        if obj.type.name != "Texture2D":
            continue
        tree = obj.read_typetree()
        stream = tree.get("m_StreamData") or {}
        if not stream.get("path") and tree["m_TextureFormat"] not in (DXT1, DXT5):
            continue
        if stream.get("path", "").startswith("archive:"):
            res_entry = stream["path"]
        data = texture_bytes(env, tree, data_dir)
        if not data:
            raise RuntimeError(f"{src}: no image data for {tree['m_Name']}")
        jobs.append((tree["m_TextureFormat"], tree["m_Width"], tree["m_Height"], tree["m_MipCount"],
                     tree.get("m_ImageCount", 1), data))
        textures.append((obj, tree))

    if is_bundle and res_entry is None:
        # Only inline textures so far: give the bundle a .resS named after its CAB.
        cab = next(name for name, f in env.file.files.items() if hasattr(f, "objects"))
        res_entry = f"archive:/{cab}/{cab}.resS"
    res_path = res_entry if is_bundle else os.path.basename(src) + ".resS"
    res = bytearray()
    before = after = converted = 0
    for (obj, tree), job, out in zip(textures, jobs, pool.imap(_encode_or_keep, jobs, chunksize=4)):
        if job[0] in (DXT1, DXT5):
            before += len(job[5])
            after += len(out)
            converted += 1
            tree["m_TextureFormat"] = fmt_id
            tree["m_CompleteImageSize"] = len(out) // max(1, tree.get("m_ImageCount", 1))
        # Unity aligns stream data in .resS files to 16 bytes.
        res += b"\0" * ((-len(res)) % 16)
        tree["m_StreamData"] = {"offset": len(res), "size": len(out), "path": res_path}
        tree["image data"] = b""
        res += out
        obj.save_typetree(tree)

    if args.platform != "keep":
        _set_platform(env.file, PLATFORMS[args.platform])
    if args.platform == "android":
        for obj in env.objects:
            if obj.type.name == "PlayerSettings":
                # UnityPy's tree for this class stops 4 bytes short of the object
                # (an unknown trailing field), so carry the unread tail over as is.
                raw = obj.get_raw_data()
                tree = obj.read_typetree(check_read=False)
                tree.update(ANDROID_PLAYER_SETTINGS)
                data = obj.save_typetree(tree)
                obj.set_raw_data(data + raw[len(data):])
            elif obj.type.name == "BuildSettings":
                # The graphics APIs the player may create a device for: the PC build
                # lists OpenGLCore (17); Android needs OpenGLES3 (11).
                tree = obj.read_typetree()
                tree["m_GraphicsAPIs"] = [GLES3]
                obj.save_typetree(tree)
            elif obj.type.name == "QualitySettings":
                # The PC levels sync to the display (vSyncCount 1). The frame rate on
                # Android comes from Application.targetFrameRate instead
                # (OniMobile.FrameStats), whose default there is 30.
                tree = obj.read_typetree()
                for level in tree["m_QualitySettings"]:
                    level["vSyncCount"] = 0
                obj.save_typetree(tree)

    if is_bundle:
        if jobs:
            writer = EndianBinaryWriter()
            writer.write_bytes(bytes(res))
            name = os.path.basename(res_path)
            old = env.file.files.get(name)
            writer.flags = getattr(old, "flags", 0)  # node flags; 0 for a plain .resS
            env.file.files[name] = writer
            env.file.mark_changed()
        data = env.file.save(packer="lz4")
    else:
        data = env.file.save()
        if jobs:
            with open(dst + ".resS", "wb") as f:
                f.write(res)
    with open(dst, "wb") as f:
        f.write(data)
    print(f"{os.path.relpath(src, data_dir)}: {shaders} shaders, {converted} textures, {before >> 20} MB -> "
          f"{after >> 20} MB, resS {len(res) >> 20} MB, {time.time() - started:.0f} s", flush=True)


def _set_platform(f, platform):
    """Sets the target platform of a serialized file (also inside bundles)."""
    if isinstance(f, SerializedFile):
        f._m_target_platform = platform
        f.target_platform = BuildTarget(platform)
        f.mark_changed()
    elif isinstance(f, BundleFile):
        for child in f.files.values():
            _set_platform(child, platform)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("data_dir")
    ap.add_argument("out_dir")
    ap.add_argument("--block", default="6x6", choices=ASTC_FORMATS)
    ap.add_argument("--platform", default="keep", choices=["keep", *PLATFORMS])
    ap.add_argument("--shaders", default="keep", choices=["keep", "gles"])
    ap.add_argument("--only", nargs="*", help="convert only these files (relative to data_dir)")
    ap.add_argument("--skip-hires", action="store_true", help="leave the hires_* bundles out")
    ap.add_argument("--jobs", type=int, default=os.cpu_count())
    args = ap.parse_args()

    data_dir, out_dir = os.path.abspath(args.data_dir), os.path.abspath(args.out_dir)
    targets = [os.path.join(data_dir, f) for f in MAIN_FILES]
    streaming = os.path.join(data_dir, "StreamingAssets")
    for name in sorted(os.listdir(streaming)):
        path = os.path.join(streaming, name)
        if os.path.isfile(path):
            with open(path, "rb") as f:
                if f.read(7) == b"UnityFS":
                    if args.skip_hires and name.startswith("hires_"):
                        continue
                    targets.append(path)
    if args.only:
        wanted = {os.path.join(data_dir, n) for n in args.only}
        targets = [t for t in targets if t in wanted]

    block = tuple(int(v) for v in args.block.split("x"))
    with multiprocessing.Pool(args.jobs, initializer=_init_worker, initargs=(block,)) as pool:
        for src in targets:
            dst = os.path.join(out_dir, os.path.relpath(src, data_dir))
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            convert_file(src, dst, data_dir, args, pool)


if __name__ == "__main__":
    sys.exit(main())
