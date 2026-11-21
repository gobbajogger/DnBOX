#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Repack pinned Chicago inputs losslessly; never modify the pinned header.

The low 27 bits of each initial value and a signed initial slope anchor the
block. Second differences use zigzag or a signed 16-bit base with unsigned
offsets. Codes 24 through 31 select that base and an offset width of 0-7.
"""
import re
from statistics import median_low
import struct
import sys
from pathlib import Path


def unpack(data, offsets):
    """Read all words from the pinned 32-entry source blocks."""
    words = []
    for block, pos in enumerate(offsets):
        count = min(32, 16385 - 32 * block)
        end = offsets[block + 1] if block + 1 < len(offsets) else len(data)
        if not 0 <= pos <= end <= len(data) or end - pos < 9:
            raise ValueError("invalid input range")
        value, delta, width = struct.unpack_from("<IiB", data, pos)
        if width > 31 or end-pos != 9 + (max(count-2, 0)*width+7)//8:
            raise ValueError("invalid residual layout")
        payload = int.from_bytes(data[pos + 9:end], "little")
        words.append(value)
        for i in range(1, count):
            if i > 1:
                code = payload & ((1 << width) - 1)
                payload >>= width
                delta += (code >> 1) ^ -(code & 1)
            value = (value + delta) & 0xffffffff
            words.append(value)
    return words


def rice_block(first, delta, second, width):
    """Use unary quotient and fixed-width remainder only when the block shrinks."""
    if not second:
        return None
    best = None
    for bias in {0, median_low(second)}:
        if not -32768 <= bias <= 32767:
            continue
        codes = [2*(x-bias) if x >= bias else 2*(bias-x)-1 for x in second]
        for k in range(min(width, 16)):
            bit_count = sum((x >> k) + 1 + k for x in codes)
            length = 8 + 2 * (bias != 0) + (bit_count + 7) // 8
            if best is None or length < best[0]:
                best = (length, bias, k, codes)
    if best is None:
        return None
    _, bias, k, codes = best
    out = bytearray((first & 0xffffff).to_bytes(3, "little"))
    out.append((first >> 24 & 7) << 5 | 31)
    out.extend(delta.to_bytes(3, "little", signed=True))
    out.append(k | ((bias != 0) << 4))
    if bias:
        out.extend(bias.to_bytes(2, "little", signed=True))
    bits = pos = 0
    for code in codes:
        pos += code >> k
        bits |= 1 << pos
        bits |= (code & ((1 << k) - 1)) << (pos + 1)
        pos += k + 1
    out.extend(bits.to_bytes((pos + 7) // 8, "little"))
    return out


def pack_blocks(words, block_size, prefix, use_rice=False):
    """Encode every word exactly and keep an offset for each random-access block."""
    packed = bytearray()
    positions = []
    for pos in range(0, len(words), block_size):
        row = words[pos:pos + block_size]
        if row[0] & 0xf8000000 != prefix:
            raise ValueError("inconsistent Chicago value prefix")
        delta = [b - a for a, b in zip(row, row[1:])]
        second = [b - a for a, b in zip(delta, delta[1:])]
        unsigned = [(2 * x) if x >= 0 else -2 * x - 1 for x in second]
        width = max(unsigned, default=0).bit_length()
        if width >= 24:
            raise ValueError("wide Chicago second difference")
        field, bias = width, None
        if second:
            lo = min(second)
            span = (max(second) - lo).bit_length()
            if -32768 <= lo <= 32767 and span <= (6 if use_rice else 7) and (width-span)*len(second) > 16:
                field, width, bias = 24 + span, span, lo
                unsigned = [x - lo for x in second]
        first_delta = delta[0] if delta else 0
        if not -(1 << 23) <= first_delta < (1 << 23):
            raise ValueError("wide Chicago initial slope")
        rice = rice_block(row[0], first_delta, second, max((x.bit_length() for x in unsigned), default=0)+1) if use_rice else None
        positions.append(len(packed))
        packed.extend((row[0] & 0xffffff).to_bytes(3, "little"))
        packed.append(((row[0] >> 24) & 7) << 5 | field)
        size = next(n for n in (1, 2, 3) if -(1 << (8*n-1)) <= first_delta < (1 << (8*n-1)))
        packed.extend(first_delta.to_bytes(size, "little", signed=True))
        if bias is not None:
            packed.extend(bias.to_bytes(2, "little", signed=True))
        acc = sum(x << (width * i) for i, x in enumerate(unsigned))
        packed.extend(acc.to_bytes((len(unsigned)*width + 7)//8, "little"))
        if rice is not None and len(rice) < len(packed) - positions[-1]:
            packed[positions[-1]:] = rice
    return packed, positions


def generate(source, output):
    text = Path(source).read_text()
    lines = ['/* Generated private Chicago input; do not commit/distribute. */',
             '#include "lossless_table.h"',
             '#if X0X_SUPERJ6_FACTORY', '#define CHICAGO_PACKED_SHIFT 8',
             '#else', '#define CHICAGO_PACKED_SHIFT 6', '#endif']
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
        words = unpack(data, offsets)
        prefix = words[0] & 0xf8000000
        for shift in (8, 6):
            if shift == 8:
                lines.append("#if X0X_SUPERJ6_FACTORY")
            else:
                lines.append("#else")
            packed, new_offsets = pack_blocks(words, 1 << shift, prefix, shift == 8)
            if len(packed) > 65535:
                raise ValueError("offset overflow")
            for suffix, values, ctype in [('data', packed, 'uint8_t'), ('offset', new_offsets, 'uint16_t')]:
                lines.append('static const %s %s_%s[%d]={' % (ctype, stem, suffix, len(values)))
                lines.extend(','.join(map(str, values[i:i+24])) + ',' for i in range(0, len(values), 24))
                lines.append('};')
            lines.append('#define %s_prefix 0x%08xu' % (stem, prefix))
            lines.append('static const original_lossless_table16 %s={%s_data,%s_offset,sizeof %s_data,16385,%d};' % (stem, stem, stem, stem, len(new_offsets)))
        lines.append("#endif")
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text('\n'.join(lines) + '\n')

if __name__ == '__main__':
    generate(sys.argv[1], sys.argv[2])
