"""Prints layout.txt blocks for C# structs, with offsets computed like the CLR does for
blittable sequential structs (see gen_gdu_layout.py).

Usage: cs_layout.py <decompiled C# root> <struct name>...
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gen_gdu_layout import PRIMS, index_types, load_sources, size_align

TYPES = {
    'byte': 'u8', 'sbyte': 'i8', 'bool': 'bool', 'short': 'i16', 'ushort': 'u16', 'int': 'i32',
    'uint': 'u32', 'float': 'f32', 'long': 'i64', 'ulong': 'u64', 'double': 'f64', 'IntPtr': 'ptr',
    'HashedString': 'u32',
}


def layout_type(t, enums):
    t = t.split('.')[-1]
    if t.endswith('*'):
        return 'ptr'
    if t in TYPES:
        return TYPES[t]
    if t in enums:
        return {1: 'u8', 2: 'u16', 4: 'i32', 8: 'i64'}[enums[t]]
    return t


def main(root, names):
    structs, enums = index_types(load_sources(root))
    for name in names:
        if name not in structs:
            print(f'# {name}: no C# struct of that name\n')
            continue
        pack, fields = structs[name]
        size, _ = size_align(name, structs, enums)
        print(f'struct {name} {size:#x}' + (f' pack={pack}' if pack != 8 else ''))
        off = 0
        for ft, fn in fields:
            s, a = size_align(ft, structs, enums)
            a = min(a, pack)
            off = (off + a - 1) // a * a
            print(f'  {off:#04x} {layout_type(ft, enums)} {fn}')
            off += s
        print()


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2:])
