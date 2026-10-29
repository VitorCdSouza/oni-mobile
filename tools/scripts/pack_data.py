"""Pack player data files into one LZ4HC UnityFS archive, the data.unity3d a Unity build
makes with BuildOptions.CompressWithLz4HC.

The player reads the archive in place from the APK and decompresses the 128 KB block it
needs, so the APK stores it uncompressed while its content takes about a third of the
space (the ASTC atlases are mostly empty space, the serialized files compress to ~25%).
The archive is written in one pass: header, blocks, then the block and directory info
at the end (the layout of the game's own asset bundles).

Usage: pack_data.py <out data.unity3d> <name=path>...
       name is the path inside the archive (sharedassets0.assets, Resources/unity_builtin_extra).
"""

import struct
import sys

import lz4.block

BLOCK = 128 * 1024
UNITY_VERSION = b"6000.3.5f2"

# ArchiveFlags (UnityPy enums/BundleFile.py) and the storage block and node flags.
LZ4HC = 3
COMBINED = 0x40
INFO_AT_END = 0x80
SERIALIZED_FILE = 4
RAW_EXTENSIONS = (".resS", ".resource", ".json", ".config")


def main():
    out_path, items = sys.argv[1], sys.argv[2:]
    blocks, nodes = [], []
    raw = 0
    with open(out_path, "wb") as out:
        header = b"UnityFS\0" + struct.pack(">I", 8) + b"5.x.x\0" + UNITY_VERSION + b"\0"
        size_at = len(header)
        header += struct.pack(">qIII", 0, 0, 0, 0)
        header += b"\0" * (-len(header) % 16)
        out.write(header)

        # Files are concatenated into one stream that is cut into blocks, as Unity does.
        offset = 0
        pending = bytearray()
        for item in items:
            name, path = item.split("=", 1)
            start = offset
            with open(path, "rb") as f:
                while True:
                    chunk = f.read(BLOCK * 64)
                    if not chunk:
                        break
                    offset += len(chunk)
                    pending += chunk
                    used = 0
                    while len(pending) - used >= BLOCK:
                        write_block(out, bytes(pending[used:used + BLOCK]), blocks)
                        used += BLOCK
                    del pending[:used]
            flags = 0 if name.endswith(RAW_EXTENSIONS) else SERIALIZED_FILE
            nodes.append((start, offset - start, flags, name))
            raw += offset - start
        if pending:
            write_block(out, bytes(pending), blocks)

        info = b"\0" * 16 + struct.pack(">i", len(blocks))
        info += b"".join(struct.pack(">IIH", u, c, f) for u, c, f in blocks)
        info += struct.pack(">i", len(nodes))
        info += b"".join(struct.pack(">qqI", o, s, f) + n.encode() + b"\0" for o, s, f, n in nodes)
        packed = lz4.block.compress(info, mode="high_compression", compression=12, store_size=False)
        out.write(packed)
        total = out.tell()
        out.seek(size_at)
        out.write(struct.pack(">qIII", total, len(packed), len(info), LZ4HC | COMBINED | INFO_AT_END))
    print(f"{out_path}: {len(nodes)} files, {raw >> 20} MB in {total >> 20} MB, {len(blocks)} blocks")


def write_block(out, data, blocks):
    packed = lz4.block.compress(data, mode="high_compression", compression=9, store_size=False)
    if len(packed) < len(data):
        out.write(packed)
        blocks.append((len(data), len(packed), LZ4HC))
    else:
        out.write(data)
        blocks.append((len(data), len(data), 0))


if __name__ == "__main__":
    main()
