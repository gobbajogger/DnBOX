#!/usr/bin/env python3
"""Exact build-local Chicago factory parameter dictionary and column layout."""
import re
import struct
import sys
from pathlib import Path

N, M = 81, 22

def pack(text):
    def literals(n):
        return re.search(n + r'\[\d+\]=\{(.*?)\}', text, re.S)[1].split(',')
    def f32(x):
        return struct.pack('<f', x)
    values = [float.fromhex(v.strip()[:-1])
              for v in literals('original_chicago_factory_dictionary')]
    baseline = bytes(int(v) for v in literals('original_chicago_factory_codes'))
    grid = {f32(i / 127): i for i in range(128)}
    assert len(values) == 309
    numerators, raw, ranks = [], [], []
    for v in values:
        exact = f32(v)
        n = round(v * 1000)
        if exact in grid:
            ranks.append(grid[exact])
        elif exact == f32(n / 1000):
            ranks.append(128 + len(numerators))
            numerators.append(n)
        else:
            ranks.append(-1 - len(raw))
            raw.append(struct.unpack('<I', exact)[0])
    ranks = [128 + len(numerators) - r - 1 if r < 0 else r for r in ranks]
    assert min(ranks) == 0 and max(ranks) == 308
    integer = int.from_bytes(baseline,'little')
    cols = [[ranks[(integer >> (9 * (i*M+j))) & 511] for i in range(N)]
            for j in range(M)]

    def bits(vs, w):
        return sum(x << (w*i) for i,x in enumerate(vs)).to_bytes((len(vs)*w+7)//8,'little')

    data, desc = bytearray(), []
    for col in cols:
        palette = sorted(set(col))
        width = (len(palette) - 1).bit_length()
        raw_col = bits(col,9)
        # For small columns only: exact local palette, then uniform rank bits.
        codes = bytes([len(palette)]) + b''.join(struct.pack('<H', x) for x in palette)
        local = codes + bits([palette.index(x) for x in col], width)
        if len(local) >= len(raw_col):
            width, payload = 9, raw_col
        else:
            payload = local
        assert len(data) < 4096
        desc.append((width << 12) | len(data))
        data += payload
    data.append(0)  # padded word reads across the final column edge
    grid1000 = bits(numerators,10) + b'\x00\x00\x00'
    grid1000 += b'\x00' * ((-len(grid1000)) % 4)
    return desc,data,grid1000,raw,len(numerators)


def generate(src, out):
    desc, data, grid, raw, n = pack(src.read_text())
    def c_array(ctype, name, xs, width=12):
        rows = [', '.join(str(x) for x in xs[i:i+width]) for i in range(0,len(xs),width)]
        return f'static const {ctype} {name}[] = {{\n' + ''.join('    '+row+',\n' for row in rows) + '};\n'
    text = '/* Build-local private exact coefficients; do not distribute. */\n'
    text += f'#define CHICAGO_GRID1000_N {n}u\n'
    text += c_array('uint16_t','chi_factory_cols',desc)
    text += c_array('uint8_t','chi_factory_codes',data)
    text += c_array('uint32_t','chi_factory_grid',list(struct.unpack(f'<{len(grid)//4}I',grid)))
    text += c_array('uint32_t','chi_factory_raw',raw)
    out.write_text(text)
    print('Chicago factory cols: exact aggregate bytes',2*len(desc)+len(data)+len(grid)+4*len(raw))

if __name__ == '__main__':
    generate(*map(Path,sys.argv[1:]))
