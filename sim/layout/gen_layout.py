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
        'i64': ('int64_t', 8), 'f64': ('double', 8), 'ptr': ('void *', 8),
        'mutex': ('uint8_t', 40)}  # a pthread_mutex_t: 40 bytes in the original (glibc x86_64)


def parse(path):
    structs, order, cur = {}, [], None
    globals_ = []
    for raw in open(path):
        line = raw.split('#', 1)[0].rstrip()
        if not line.strip():
            continue
        if line.startswith('global '):
            _, name, typ = line.split()
            globals_.append((name, typ))
            continue
        if line.startswith('struct '):
            parts = line.split()
            name, size = parts[1], parts[2]
            pack = next((int(p.split('=')[1]) for p in parts[3:] if p.startswith('pack=')), None)
            cur = {'name': name, 'size': int(size, 16), 'fields': [], 'methods': [], 'pack': pack}
            structs[name] = cur
            order.append(name)
            continue
        if line.strip().startswith('method '):
            cur['methods'].append(line.strip()[len('method '):])
            continue
        off, typ, fname = line.split()
        cur['fields'].append((int(off, 16), typ, fname))
    parse.globals = globals_
    return structs, order


def size_of(typ, structs):
    m = re.fullmatch(r'(.+)\[(\d+)\]', typ)
    if m:
        return size_of(m.group(1), structs) * int(m.group(2))
    if typ.startswith('queue<'):
        return 0x50  # std::queue over a libstdc++ std::deque
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


def embedded_first(structs, order):
    """Orders structs so that a struct embedded by value is defined before its container."""
    done, result = set(), []

    def visit(name):
        if name in done:
            return
        done.add(name)
        for _, typ, _ in structs[name]['fields']:
            base = re.sub(r'\[\d+\]$', '', typ)
            if base in structs:
                visit(base)
        result.append(name)

    for name in order:
        visit(name)
    return result


def ghidra_header(structs, order):
    order = embedded_first(structs, order)
    out = ['// Generated from sim/layout/layout.txt. Do not edit.', 'typedef unsigned char uint8_t;',
           'typedef unsigned short uint16_t;', 'typedef short int16_t;', 'typedef unsigned int uint32_t;',
           'typedef int int32_t;', 'typedef unsigned long long uint64_t;', 'typedef long long int64_t;', '']
    vecs = {}
    all_types = [typ for name in order for _, typ, _ in structs[name]['fields']]
    all_types += [typ for _, typ in parse.globals]
    for name in [None]:
        for typ in all_types:
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
        if s['pack']:
            out.append(f'#pragma pack(push, {s["pack"]})')
        out.append(f'struct {name} {{')
        pos = 0
        for off, typ, fname in sorted(s['fields']):
            if off < pos:
                raise SystemExit(f'{name}.{fname}: offset {off:#x} overlaps previous field')
            if off > pos:
                out.append(f'    uint8_t pad_{pos:#x}[{off - pos}];')
            m = re.fullmatch(r'(.+)\[(\d+)\]', typ)
            base, count = (m.group(1), m.group(2)) if m else (typ, None)
            if base == 'mutex':
                out.append(f'    uint8_t {fname}[40];')
                pos = off + size_of(typ, structs)
                continue
            if base.startswith('queue<'):
                out.append(f'    uint8_t {fname}[80];')
                pos = off + size_of(typ, structs)
                continue
            t = f'struct {vec_struct_name(base[4:-1])}' if base.startswith('vec<') else c_type(base)
            out.append(f'    {t} {fname}' + (f'[{count}]' if count else '') + ';')
            pos = off + size_of(typ, structs)
        if pos > s['size']:
            raise SystemExit(f'{name}: fields end at {pos:#x}, past size {s["size"]:#x}')
        if pos < s['size']:
            out.append(f'    uint8_t pad_{pos:#x}[{s["size"] - pos}];')
        out.append('};')
        if s['pack']:
            out.append('#pragma pack(pop)')
        out.append('')
    # Global variable types, read by ApplyTypes.java: "//@global <symbol> <C type name> <is pointer>".
    for gname, typ in parse.globals:
        if typ.startswith('vec<'):
            out.append(f'//@global {gname} {vec_struct_name(typ[4:-1])} 0')
        elif typ.endswith('*'):
            out.append(f'//@global {gname} {typ[:-1]} 1')
        else:
            out.append(f'//@global {gname} {typ} 0')
    return '\n'.join(out)


CPP_PRIM = {'u8': 'uint8_t', 'bool': 'bool', 'u16': 'uint16_t', 'i16': 'int16_t', 'u32': 'uint32_t',
            'i32': 'int32_t', 'f32': 'float', 'u64': 'uint64_t', 'i64': 'int64_t', 'f64': 'double', 'ptr': 'void *'}


def cpp_type(typ):
    if typ.startswith('queue<'):
        return f'std::queue<{cpp_type(typ[6:-1])}>'
    if typ.startswith('vec<'):
        return f'std::vector<{cpp_type(typ[4:-1])}>'
    if typ.endswith('*'):
        return cpp_type(typ[:-1]) + ' *'
    return CPP_PRIM.get(typ, typ)


def cpp_header(structs, order):
    order = embedded_first(structs, order)
    out = ['// Generated from sim/layout/layout.txt. Do not edit.',
           '// The original SimDLL structures, laid out exactly like the original.',
           '#pragma once', '#include <pthread.h>', '#include <cstddef>', '#include <cstdint>', '#include <mutex>',
           '#include <queue>', '#include <vector>', '',
           '// Room for a std::mutex / pthread_mutex_t: 40 bytes in the original (glibc x86_64), more',
           '// on other platforms (48 on glibc aarch64). Only x86_64 builds keep the original layout.',
           'constexpr size_t kMutexBytes = sizeof(pthread_mutex_t) > sizeof(std::mutex) ? sizeof(pthread_mutex_t)',
           '                                                                               : sizeof(std::mutex);',
           '',
           '// Enums used in member function signatures (they shape the mangled names).',
           'namespace SpawnFXHashes { enum Type : int32_t {}; }', '',
           '// The original\'s small vector templates (their names appear in mangled signatures).',
           'template <typename T>', 'struct Vector2 {', '    T x, y;', '};',
           'template <typename T>', 'struct Vector4 {', '    T x, y, z, w;', '};', '',
           '// Component handle: low 24 bits index, top 8 bits version; 0xf00000 set means invalid.',
           '// Its copy constructor is user-provided, as in the original: that makes it non-trivial for',
           '// the ABI, so it is returned through a hidden pointer and passed by invisible reference.',
           'struct Handle {',
           '    uint32_t value;',
           '    Handle() = default;',
           '    explicit Handle(uint32_t v) : value(v) {}',
           '    Handle(const Handle &other) : value(other.value) {}',
           '    Handle &operator=(const Handle &other) { value = other.value; return *this; }',
           '};', '']
    out += [f'struct {n};' for n in order] + ['']
    for name in order:
        s = structs[name]
        if s['pack']:
            out.append(f'#pragma pack(push, {s["pack"]})')
        out.append(f'struct {name} {{')
        pos = 0
        for off, typ, fname in sorted(s['fields']):
            if off > pos:
                out.append(f'    uint8_t pad_{pos:#x}[{off - pos}];')
            m = re.fullmatch(r'(.+)\[(\d+)\]', typ)
            base, count = (m.group(1), m.group(2)) if m else (typ, None)
            if base == 'mutex':
                out.append(f'    alignas(8) uint8_t {fname}[kMutexBytes];')
                pos = off + size_of(typ, structs)
                continue
            out.append(f'    {cpp_type(base)} {fname}' + (f'[{count}]' if count else '') + ';')
            pos = off + size_of(typ, structs)
        if pos < s['size']:
            out.append(f'    uint8_t pad_{pos:#x}[{s["size"] - pos}];')
        for m in s['methods']:
            out.append(f'    {m}')
        out.append('};')
        if s['pack']:
            out.append('#pragma pack(pop)')
        out.append('')
    for gname, typ in parse.globals:
        out.append(f'extern {cpp_type(typ)} {gname};')
    out.append('')
    out.append('#include "layout_checks.h"')
    return '\n'.join(out)


def checks_header(structs, order):
    out = ['// Generated from sim/layout/layout.txt. Do not edit.',
           '// Included after the port\'s struct definitions. The layout only has to match the original',
           '// where the hybrid build runs against it (x86_64 with libstdc++).',
           '#pragma once', '#include <cstddef>', '', '#if defined(__x86_64__) && defined(__GLIBCXX__)', '']
    for name in order:
        s = structs[name]
        out.append(f'static_assert(sizeof({name}) == {s["size"]:#x}, "{name} size");')
        for off, _, fname in s['fields']:
            if not fname.startswith('unk_'):
                out.append(f'static_assert(offsetof({name}, {fname}) == {off:#x}, "{name}::{fname}");')
        out.append('')
    out.append('#endif')
    return '\n'.join(out)


def main(src, ghidra_out, checks_out, cpp_out=None):
    structs, order = parse(src)
    if cpp_out:
        open(cpp_out, 'w').write(cpp_header(structs, order) + '\n')
    open(ghidra_out, 'w').write(ghidra_header(structs, order) + '\n')
    open(checks_out, 'w').write(checks_header(structs, order) + '\n')


if __name__ == '__main__':
    main(*sys.argv[1:5])
