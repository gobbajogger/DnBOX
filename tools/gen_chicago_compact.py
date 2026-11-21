#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Repack pinned Chicago inputs losslessly; never modify the pinned header.

Each 32-entry block retains its exact residual payload and the smallest signed
first difference that covers its original value. The directory uses one base
and stride per sixteen offsets, plus a signed residual per block.
"""
import re
import struct
import sys
from pathlib import Path


def directory(offsets):
    bases, strides, residuals = [], [], []
    for start in range(0, len(offsets), 16):
        group = offsets[start:start + 16]
        stride = round((group[-1] - group[0]) / (len(group) - 1)) if len(group) > 1 else 0
        residual = [position - (group[0] + i * stride) for i, position in enumerate(group)]
        if not 0 <= stride <= 255 or any(not -128 <= n <= 127 for n in residual):
            raise ValueError('offset group exceeds its lossless representation')
        bases.append(group[0])
        strides.append(stride)
        residuals.extend(residual)
    return bases, strides, residuals


def generate(source, output):
    text = Path(source).read_text()
    lines = ['/* Generated private Chicago input; do not commit/distribute. */',
             '#include "lossless_table.h"']
    for name in ('a', 'e'):
        stem = 'original_chicago_packed_' + name
        def array(suffix):
            match = re.search(stem + '_' + suffix + r'\[\d+\]=\{(.*?)\};', text, re.S)
            if not match:
                raise ValueError('missing input array')
            return [int(x) for x in match[1].split(',') if x.strip()]
        data, offsets = bytes(array('data')), array('offset')
        if len(offsets) != 513 or offsets[0] != 0:
            raise ValueError('unexpected Chicago layout')
        packed, new_offsets, prefix = bytearray(), [], None
        for block, offset in enumerate(offsets):
            end = offsets[block+1] if block+1 < len(offsets) else len(data)
            if not 0 <= offset <= end <= len(data) or end-offset < 9:
                raise ValueError('invalid input range')
            value, delta, width = struct.unpack_from('<IiB', data, offset)
            entries = min(32, 16385 - block*32)
            if width > 31 or end-offset != 9 + (max(entries-2, 0)*width+7)//8:
                raise ValueError('invalid residual layout')
            if prefix is None:
                prefix = value & 0xf8000000
            if value & 0xf8000000 != prefix or not -(1<<23) <= delta < (1<<23):
                raise ValueError('header cannot be represented losslessly')
            new_offsets.append(len(packed))
            packed.extend((value & 0xffffff).to_bytes(3, 'little'))
            packed.append(((value >> 24) & 7) << 5 | width)
            size = next(n for n in (1, 2, 3) if -(1 << (8*n-1)) <= delta < (1 << (8*n-1)))
            packed.extend(delta.to_bytes(size, 'little', signed=True))
            packed.extend(data[offset+9:end])
        if len(packed) > 65535:
            raise ValueError('offset overflow')
        bases, strides, residuals = directory(new_offsets)
        for suffix, values, ctype in [('data', packed, 'uint8_t'), ('base', bases, 'uint16_t'),
                                      ('stride', strides, 'uint8_t'), ('residual', residuals, 'int8_t')]:
            lines.append('static const %s %s_%s[%d]={' % (ctype, stem, suffix, len(values)))
            lines.extend(','.join(map(str, values[i:i+24])) + ',' for i in range(0, len(values), 24))
            lines.append('};')
        lines.append('#define %s_prefix 0x%08xu' % (stem, prefix))
        lines.append(f'static const chicago_compact_table {stem}={{'
                     f'{stem}_data,{stem}_base,{stem}_stride,{stem}_residual,sizeof {stem}_data}};')
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text('\n'.join(lines) + '\n')

if __name__ == '__main__':
    generate(sys.argv[1], sys.argv[2])
