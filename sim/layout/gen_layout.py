"""Generates the two views of sim/layout/layout.txt.

    python3 gen_layout.py layout.txt <ghidra_types.h> <layout_checks.h>

ghidra_types.h: plain C structs for Ghidra's C parser (vectors become begin/end/cap pointer
triples, gaps become padding arrays), applied to the program by ApplyTypes.java.

layout_checks.h: static_asserts that the port's C++ structs put every named field at the
original offset and have the original size, so hybrid mode can share memory with the original.
Fields named unk_* are skipped in the checks; the port does not need to name them.
"""
import re
import sys

PRIM = {'u8': ('uint8_t', 1), 'bool': ('uint8_t', 1), 'u16': ('uint16_t', 2), 'i16': ('int16_t', 2),
        'u32': ('uint32_t', 4), 'i32': ('int32_t', 4), 'f32': ('float', 4), 'u64': ('uint64_t', 8),
        'i64': ('int64_t', 8), 'f64': ('double', 8), 'ptr': ('void *', 8)}


def parse(path):
    structs, order, cur = {}, [], None
    for raw in open(path):
        line = raw.split('#', 1)[0].rstrip()
        if not line.strip():
            continue
        if line.startswith('struct '):
            _, name, size = line.split()
            cur = {'name': name, 'size': int(size, 16), 'fields': []}
            structs[name] = cur
            order.append(name)
            continue
        off, typ, fname = line.split()
        cur['fields'].append((int(off, 16), typ, fname))
    return structs, order


def size_of(typ, structs):
    m = re.fullmatch(r'(.+)\[(\d+)\]', typ)
    if m:
        return size_of(m.group(1), structs) * int(m.group(2))
    if typ.startswith('vec<') or typ.endswith('*'):
        return 24 if typ.startswith('vec<') else 8
    if typ in PRIM:
        return PRIM[typ][1]
    return structs[typ]['size']


def c_type(typ):
    if typ.endswith('*'):
        base = typ[:-1]
        return (PRIM[base][0] if base in PRIM else f'struct {base}') + ' *'
    if typ in PRIM:
        return PRIM[typ][0]
    return f'struct {typ}'


def vec_struct_name(elem):
    return 'Vec_' + re.sub(r'\W', '_', elem.rstrip('*')) + ('_ptr' if elem.endswith('*') else '')


def ghidra_header(structs, order):
    out = ['// Generated from sim/layout/layout.txt. Do not edit.', 'typedef unsigned char uint8_t;',
           'typedef unsigned short uint16_t;', 'typedef short int16_t;', 'typedef unsigned int uint32_t;',
           'typedef int int32_t;', 'typedef unsigned long long uint64_t;', 'typedef long long int64_t;', '']
    vecs = {}
    for name in order:
        for _, typ, _ in structs[name]['fields']:
            base = re.sub(r'\[\d+\]$', '', typ)
            if base.startswith('vec<'):
                vecs[vec_struct_name(base[4:-1])] = base[4:-1]
    out += [f'struct {n};' for n in order] + ['']
    for vname, elem in sorted(vecs.items()):
        t = c_type(elem if elem.endswith('*') or elem in PRIM else elem)
        out.append(f'struct {vname} {{ {t} *begin; {t} *end; {t} *cap; }};')
    out.append('')
    for name in order:
        s = structs[name]
        out.append(f'struct {name} {{')
        pos = 0
        for off, typ, fname in sorted(s['fields']):
            if off < pos:
                raise SystemExit(f'{name}.{fname}: offset {off:#x} overlaps previous field')
            if off > pos:
                out.append(f'    uint8_t pad_{pos:#x}[{off - pos}];')
            m = re.fullmatch(r'(.+)\[(\d+)\]', typ)
            base, count = (m.group(1), m.group(2)) if m else (typ, None)
            t = f'struct {vec_struct_name(base[4:-1])}' if base.startswith('vec<') else c_type(base)
            out.append(f'    {t} {fname}' + (f'[{count}]' if count else '') + ';')
            pos = off + size_of(typ, structs)
        if pos > s['size']:
            raise SystemExit(f'{name}: fields end at {pos:#x}, past size {s["size"]:#x}')
        if pos < s['size']:
            out.append(f'    uint8_t pad_{pos:#x}[{s["size"] - pos}];')
        out.append('};')
        out.append('')
    return '\n'.join(out)


def checks_header(structs, order):
    out = ['// Generated from sim/layout/layout.txt. Do not edit.',
           '// Included by the port in hybrid builds, after its own struct definitions.',
           '#pragma once', '#include <cstddef>', '']
    for name in order:
        s = structs[name]
        out.append(f'static_assert(sizeof({name}) == {s["size"]:#x}, "{name} size");')
        for off, _, fname in s['fields']:
            if not fname.startswith('unk_'):
                out.append(f'static_assert(offsetof({name}, {fname}) == {off:#x}, "{name}::{fname}");')
        out.append('')
    return '\n'.join(out)


def main(src, ghidra_out, checks_out):
    structs, order = parse(src)
    open(ghidra_out, 'w').write(ghidra_header(structs, order) + '\n')
    open(checks_out, 'w').write(checks_header(structs, order) + '\n')


if __name__ == '__main__':
    main(*sys.argv[1:4])
