"""Replaces arm64 hint instructions the Android emulator's NDK translation does not know
(SEVL, WFE, WFI) with NOP, in the executable sections of the given ELF files.

Only for the emulator copy of the APK (tools/scripts/make_emulator_apk.sh). The hints are
allowed to do nothing: a WFE that never waits is a valid implementation, so a spinlock
built on SEVL/WFE turns into a plain spin.

    python3 emu_patch_hints.py lib1.so [lib2.so ...]
"""
import struct
import sys

NOP = 0xD503201F
HINTS = {0xD50320BF: "sevl", 0xD503205F: "wfe", 0xD503207F: "wfi"}
SHF_EXECINSTR = 0x4


def patch(path):
    with open(path, "rb") as f:
        data = bytearray(f.read())
    if data[:4] != b"\x7fELF" or data[4] != 2 or data[5] != 1:
        raise SystemExit(f"{path}: not a little-endian ELF64 file")
    shoff, = struct.unpack_from("<Q", data, 0x28)
    shentsize, shnum = struct.unpack_from("<HH", data, 0x3A)
    counts = dict.fromkeys(HINTS.values(), 0)
    for i in range(shnum):
        base = shoff + i * shentsize
        _, sh_type, flags, _, offset, size = struct.unpack_from("<IIQQQQ", data, base)
        if sh_type == 8 or not flags & SHF_EXECINSTR:  # SHT_NOBITS
            continue
        end = offset + size
        for word, name in HINTS.items():
            needle = struct.pack("<I", word)
            at = data.find(needle, offset, end)
            while at >= 0:
                # Instructions are 4-byte aligned from the section start.
                if (at - offset) % 4 == 0:
                    struct.pack_into("<I", data, at, NOP)
                    counts[name] += 1
                at = data.find(needle, at + 1, end)
    with open(path, "wb") as f:
        f.write(data)
    print(path, " ".join(f"{k}={v}" for k, v in counts.items()))


for p in sys.argv[1:]:
    patch(p)
