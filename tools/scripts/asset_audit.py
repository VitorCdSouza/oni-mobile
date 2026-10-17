"""Inventory the game's Unity assets for the Android port.

Lists every Texture2D (format, size, bytes), every Shader (name, compiled
platforms) and a per-type object count, for the main data files and the
AssetBundles in StreamingAssets. Writes a JSON report and prints a summary.

Usage: tools/venv/bin/python -I tools/scripts/asset_audit.py <Game_Data dir> <out.json>
"""

import collections
import json
import os
import sys

import UnityPy

# ShaderCompilerPlatform enum values (Runtime/Graphics/ShaderCompilerTypes.h).
SHADER_PLATFORMS = {
    0: "None", 1: "D3D9", 2: "XBOX360", 3: "PS3", 4: "D3D11", 5: "GLES20",
    6: "NaCl", 7: "Flash", 8: "D3D11_9x", 9: "GLES3Plus", 10: "PSP2",
    11: "PS4", 12: "XboxOne", 13: "PSM", 14: "Metal", 15: "OpenGLCore",
    16: "N3DS", 17: "WiiU", 18: "Vulkan", 19: "Switch", 20: "XboxOneD3D12",
    21: "GameCoreXboxOne", 22: "GameCoreScarlett", 23: "PS5", 24: "PS5NGGC",
    25: "WebGPU",
}


def unity_files(data_dir):
    for name in sorted(os.listdir(data_dir)):
        path = os.path.join(data_dir, name)
        if os.path.isfile(path) and (name.endswith(".assets") or name in ("globalgamemanagers", "level0", "level1", "level2")):
            yield path
    streaming = os.path.join(data_dir, "StreamingAssets")
    for root, _, files in os.walk(streaming):
        for name in sorted(files):
            path = os.path.join(root, name)
            with open(path, "rb") as f:
                if f.read(7) == b"UnityFS":
                    yield path


def texture_info(data):
    fmt = data.m_TextureFormat
    return {
        "name": data.m_Name,
        "format": fmt.name if hasattr(fmt, "name") else str(fmt),
        "width": data.m_Width,
        "height": data.m_Height,
        "mips": getattr(data, "m_MipCount", 1),
        "bytes": getattr(data, "m_CompleteImageSize", 0),
        "readable": bool(getattr(data, "m_IsReadable", False)),
    }


def shader_info(obj):
    tree = obj.read_typetree()
    parsed = tree.get("m_ParsedForm") or {}
    platforms = tree.get("platforms") or []
    return {
        "name": parsed.get("m_Name") or tree.get("m_Name", ""),
        "platforms": [SHADER_PLATFORMS.get(p, str(p)) for p in platforms],
        "subshaders": len(parsed.get("m_SubShaders") or []),
        "blob_bytes": sum(tree.get("compressedLengths", [[0]])[0]) if tree.get("compressedLengths") else 0,
    }


def main():
    data_dir, out_path = sys.argv[1], sys.argv[2]
    report = {"files": []}
    for path in unity_files(data_dir):
        rel = os.path.relpath(path, data_dir)
        entry = {"file": rel, "counts": {}, "textures": [], "shaders": [], "errors": []}
        try:
            env = UnityPy.load(path)
        except Exception as e:  # noqa: BLE001 - report and keep going
            entry["errors"].append(f"load: {e}")
            report["files"].append(entry)
            continue
        counts = collections.Counter()
        for obj in env.objects:
            tname = obj.type.name
            counts[tname] += 1
            try:
                if tname == "Texture2D":
                    entry["textures"].append(texture_info(obj.read()))
                elif tname == "Shader":
                    entry["shaders"].append(shader_info(obj))
            except Exception as e:  # noqa: BLE001
                entry["errors"].append(f"{tname} {obj.path_id}: {e}")
        entry["counts"] = dict(counts.most_common())
        report["files"].append(entry)
        print(f"{rel}: {sum(counts.values())} objects, {len(entry['textures'])} textures, "
              f"{len(entry['shaders'])} shaders, {len(entry['errors'])} errors", flush=True)
    with open(out_path, "w") as f:
        json.dump(report, f, indent=1)


if __name__ == "__main__":
    main()
