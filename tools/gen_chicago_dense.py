#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Encode the pinned Chicago MP4 coefficients for the combined factory profile."""
import re
import struct
from pathlib import Path


def encode(words):
    data = bytearray()
    offsets = []
    for start in range(0, len(words), 128):
        block = words[start:start + 128]
        deltas = [right - left for left, right in zip(block, block[1:])]
        seconds = [right - left for left, right in zip(deltas, deltas[1:])]
        minimum = min(seconds, default=0)
        width = (max(seconds, default=0) - minimum).bit_length()
        if width > 24:
            raise ValueError("second difference exceeds the decoder's bit reservoir")
        offsets.append(len(data))
        data.extend(struct.pack("<I", block[0]))
        data.extend((deltas[0] if deltas else 0).to_bytes(3, 'little', signed=True))
        data.extend(minimum.to_bytes(3, 'little', signed=True))
        data.append(width)
        packed = sum((value - minimum) << (i * width) for i, value in enumerate(seconds))
        data.extend(packed.to_bytes((len(seconds) * width + 7) // 8, 'little'))
    if len(data) > 65535:
        raise ValueError("Chicago coefficient offsets exceed 16 bits")
    return offsets, data


def generate(source, output):
    source = Path(source).read_text()
    header = ["/* Generated from the pinned 44.1 kHz input. */", "#include <stdint.h>"]
    for name in ('a', 'e'):
        match = re.search(r'\boriginal_chicago_' + name + r'44100\[16385\]\s*=\s*\{([^}]*)\}', source)
        if not match:
            raise ValueError('missing Chicago coefficient table ' + name)
        floats = [float.fromhex(token.strip().removesuffix('f'))
                  for token in match[1].split(',') if token.strip()]
        words = struct.unpack('<16385I', struct.pack('<16385f', *floats))
        offsets, data = encode(words)
        for suffix, ctype, values in [('offset', 'uint16_t', offsets), ('data', 'uint8_t', data)]:
            header.append(f'static const {ctype} dense_{name}_{suffix}[] = {{')
            for start in range(0, len(values), 24):
                header.append('    ' + ','.join(map(str, values[start:start + 24])) + ',')
            header.append('};')
    Path(output).write_text('\n'.join(header) + '\n')
