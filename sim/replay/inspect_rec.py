"""Summarizes a SimDLL recording: record counts per type and message id, in order of first use."""
import collections
import struct
import sys

NAMES = {1: 'SIM_INITIALIZE', 2: 'SIM_SHUTDOWN', 3: 'SIM_HANDLE_MESSAGE', 4: 'SIM_BEGIN_SAVE', 5: 'SIM_END_SAVE',
         6: 'TIME', 7: 'RESULT', 10: 'CTM_INITIALIZE', 11: 'CTM_SHUTDOWN', 12: 'CTM_ADD', 13: 'CTM_SET',
         14: 'CTM_REMOVE', 15: 'CTM_UPDATE', 16: 'CTM_CLEAR', 20: 'CALLBACK'}
FIXED = {1: 0, 2: 0, 4: 12, 5: 0, 6: 8, 7: 1, 10: 0, 11: 0, 12: 29, 13: 20, 14: 4, 15: 5, 16: 0, 20: 4}


def main(path):
    counts = collections.Counter()
    msg_counts = collections.Counter()
    msg_bytes = collections.Counter()
    order = []
    with open(path, 'rb') as f:
        assert f.read(8) == b'ONISIMR1'
        while True:
            t = f.read(1)
            if not t:
                break
            t = t[0]
            name = NAMES.get(t, f'?{t}')
            counts[name] += 1
            if name not in order:
                order.append(name)
            if t == 3:
                msg_id, length = struct.unpack('<ii', f.read(8))
                f.seek(max(length, 0), 1)
                if msg_id == -930289787:
                    (n,) = struct.unpack('<i', f.read(4))
                    f.seek(n * 32, 1)
                key = f'msg {msg_id}'
                if key not in order:
                    order.append(key)
                msg_counts[key] += 1
                msg_bytes[key] += length
            elif t in FIXED:
                f.seek(FIXED[t], 1)
            else:
                print('unknown record type', t, 'at', f.tell() - 1)
                return
    for k in order:
        if k.startswith('msg '):
            print(f'  {k:16} x{msg_counts[k]:7}  {msg_bytes[k] / 1e6:9.2f} MB')
        else:
            print(f'{k:18} x{counts[k]}')


if __name__ == '__main__':
    main(sys.argv[1])
