#!/usr/bin/env python3
"""Measure lossless archival sizes for the 694 original Synth One command streams."""
import argparse
import hashlib
import json
import lzma
from pathlib import Path
import struct
import zlib


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('commands', type=Path, help='ORIGINAL-PRESET-COMMANDS.json from compile-synthone-factory.py')
    args = parser.parse_args()
    source = args.commands.read_bytes()
    source_sha = hashlib.sha256(source).hexdigest()
    if source_sha != '25a9c436f4dc7c37517c77acb632ccdf0753a885ee474e1e8f93c02b0c099a2e':
        raise ValueError('Input differs from the audited original factory commands')
    programs = json.loads(source)['cases']
    ids = bytes(k for k, _ in programs[0]['commands'])
    if len(programs) != 694 or len(ids) != 144 or any(
            bytes(k for k, _ in p['commands']) != ids for p in programs):
        raise ValueError('Incomplete or inconsistent command streams')
    words = [[struct.pack('<f', value) for _, value in p['commands']] for p in programs]
    names = b''.join(p['name'].encode() + b'\0' for p in programs)
    rows = b''.join(b''.join(row) for row in words)
    columns = b''.join(b''.join(column) for column in zip(*words))
    by_byte = b''.join(columns[byte::4] for byte in range(4))
    results = {}
    for layout, data in [('rows', rows), ('columns', columns), ('column_byte_planes', by_byte)]:
        payload = ids + names + data
        packed = lzma.compress(payload, preset=6)
        if lzma.decompress(packed) != payload:
            raise ValueError('Factory words or names failed to round-trip')
        results[layout] = dict(bytes=len(payload), xz_bytes=len(packed),
                               zlib9_bytes=len(zlib.compress(payload, 9)),
                               sha256=hashlib.sha256(payload).hexdigest())
    print(json.dumps(dict(commands_sha256=source_sha, programs=694, layouts=results), indent=2))


if __name__ == '__main__':
    main()
