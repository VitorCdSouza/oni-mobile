"""Compares two replay outputs and reports where they first diverge.

    python3 compare.py original.txt candidate.txt

Lines are matched by position. For the first line that differs, every field that differs is
listed, so the report reads like "observation 412, gdu: mass and temperature differ".
"""
import sys


def fields(line):
    parts = line.split()
    head = parts[:2]
    values = dict(p.split('=', 1) for p in parts[2:] if '=' in p)
    return head, values


def main(a_path, b_path):
    with open(a_path) as a, open(b_path) as b:
        a_lines, b_lines = a.read().splitlines(), b.read().splitlines()
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
    sys.exit(main(sys.argv[1], sys.argv[2]))
