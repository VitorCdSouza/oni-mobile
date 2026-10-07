"""Compares two replay outputs and reports where they first diverge.

    python3 compare.py original.txt candidate.txt

Lines are matched by position. For the first line that differs, every field that differs is
listed, so the report reads like "observation 412, gdu: mass and temperature differ".

Save blobs are compared by size only: the original library writes uninitialized memory into
saves (a few hundred garbage floats that change between two runs of the same recording), so
their hash is not a usable signal. Pass --strict-saves to compare them anyway.
"""
import sys


def fields(line):
    parts = line.split()
    head = parts[:2]
    values = dict(p.split('=', 1) for p in parts[2:] if '=' in p)
    return head, values


def drop_save_hash(line):
    return ' '.join(p for p in line.split() if not p.startswith('blob=')) if ' save ' in line else line


def main(a_path, b_path, strict_saves=False):
    with open(a_path) as a, open(b_path) as b:
        a_lines, b_lines = a.read().splitlines(), b.read().splitlines()
    if not strict_saves:
        a_lines, b_lines = [drop_save_hash(x) for x in a_lines], [drop_save_hash(x) for x in b_lines]
    for i, (la, lb) in enumerate(zip(a_lines, b_lines)):
        if la == lb:
            continue
        (ha, va), (hb, vb) = fields(la), fields(lb)
        diff = sorted(k for k in set(va) | set(vb) if va.get(k) != vb.get(k))
        print(f'first divergence at line {i + 1}: {" ".join(ha)} vs {" ".join(hb)}')
        for k in diff:
            print(f'  {k}: {va.get(k)} != {vb.get(k)}')
        return 1
    if len(a_lines) != len(b_lines):
        print(f'same prefix, different length: {len(a_lines)} vs {len(b_lines)} lines')
        return 1
    print(f'identical ({len(a_lines)} observations)')
    return 0


if __name__ == '__main__':
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    sys.exit(main(args[0], args[1], strict_saves='--strict-saves' in sys.argv))
