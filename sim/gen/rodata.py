"""Prints the constants stored at virtual addresses of the original library, so ported code can
use the exact same values (the decompiler rounds float literals when it prints them).

    python3 rodata.py <lib.so> <vaddr> [<vaddr> ...]     e.g. a5420
    python3 rodata.py <lib.so> --symbol <mangled name>  every rip-relative constant the function reads
"""
import re
import struct
import subprocess
import sys


def segments(data):
    phoff, = struct.unpack_from('<Q', data, 0x20)
    phentsize, phnum = struct.unpack_from('<HH', data, 0x36)
    for i in range(phnum):
        p_type, _, p_offset, p_vaddr, _, p_filesz = struct.unpack_from('<IIQQQQ', data, phoff + i * phentsize)
        if p_type == 1:  # PT_LOAD
            yield p_vaddr, p_offset, p_filesz


def main(path, addrs):
    data = open(path, 'rb').read()
    segs = list(segments(data))
    for a in addrs:
        v = int(a, 16)
        off = next((o + v - va for va, o, sz in segs if va <= v < va + sz), None)
        if off is None:
            print(f'{a}: (not file-backed: .bss, written at run time)')
            continue
        f, = struct.unpack_from('<f', data, off)
        u, = struct.unpack_from('<I', data, off)
        d, = struct.unpack_from('<d', data, off)
        print(f'{a}: float {f!r} (0x{u:08x}, hexfloat {f.hex()})  double {d!r}')


def function_constants(path, symbol):
    nm = subprocess.run(['nm', '-D', '-S', '--defined-only', path], capture_output=True, text=True).stdout
    start, size = next((int(p[0], 16), int(p[1], 16)) for p in (l.split() for l in nm.splitlines())
                       if len(p) == 4 and p[3] == symbol)
    asm = subprocess.run(['objdump', '-d', '--no-show-raw-insn', '-M', 'intel', f'--start-address={start:#x}',
                          f'--stop-address={start + size:#x}', path], capture_output=True, text=True).stdout
    seen = []
    for line in asm.splitlines():
        m = re.search(r'(\w+)\s+.*\[rip\+0x[0-9a-f]+\]\s+# ([0-9a-f]+)', line)
        if m and m.group(2) not in seen:
            seen.append(m.group(2))
            print(f'{m.group(1):8}', end=' ')
            main(path, [m.group(2)])


if __name__ == '__main__':
    if sys.argv[2] == '--symbol':
        function_constants(sys.argv[1], sys.argv[3])
    else:
        main(sys.argv[1], sys.argv[2:])
