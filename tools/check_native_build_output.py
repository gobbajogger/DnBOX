#!/usr/bin/env python3
"""Check native SMK-37 DEV build outputs before marking a build probe passed.

This is an additional build-output integrity guard, NOT an installer, release
qualification, sample preservation check or physical-device test.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import struct
import sys
import zlib

import add_ctl
import fm1pkg_make as pkg

REGIONS = {0: (b'flash.bin', None), 100: (b'ota.bin', None),
           101: (b'xdata.bin', 0x2000), 102: (b'fittab.bin', 0x1000),
           103: (b'fdata.bin', pkg.FDATA_SIZE)}

PRODUCTS = {'elite': 'SMK-37 Elite_900', 'pro': 'SMK-37 Pro_900'}


def check_firmware(path: Path, model: str) -> dict:
    if model not in PRODUCTS:
        raise ValueError('unsupported SMK-37 model')
    raw = path.read_bytes()
    product, logical = add_ctl.logical_of(raw)
    if product != PRODUCTS[model]:
        raise ValueError(f'wrong firmware model: got {product!r}, expected {PRODUCTS[model]!r}')
    # Checks flash/app-area CRC and SMK board/slot identity using the existing
    # package implementation rather than duplicating its encrypted flash parser.
    state = add_ctl._open(logical)
    if pkg.slotmap_of(state["flash"]) != pkg.SLOTMAP["smk37"]:
        raise ValueError("SMK sample slot map missing, invalid or changed")
    if len(logical) < 0x40:
        raise ValueError('package missing UFW header')
    hdr = bytearray(logical[:0x40])
    pkg.enc(hdr, 0, len(hdr))
    count = struct.unpack_from('<H', hdr, 8)[0]
    if not 2 <= count <= len(REGIONS):
        raise ValueError(f'unexpected package file count {count}')
    if pkg.crc16(hdr[2:]) != struct.unpack_from('<H', hdr, 0)[0]:
        raise ValueError('package header CRC mismatch')
    if struct.unpack_from('<I', hdr, 4)[0] != len(logical):
        raise ValueError('package length mismatch')
    start, end = 0x40, 0x40 + 0x50 * count
    if end > len(logical):
        raise ValueError('truncated package file descriptors')
    entries = logical[start:end]
    if pkg.crc16(entries) != struct.unpack_from('<H', hdr, 2)[0]:
        raise ValueError('package file-list CRC mismatch')
    names = []
    minimum_offset, previous_kind = 0x400, -1
    for index in range(count):
        entry = bytearray(entries[index * 0x50:(index + 1) * 0x50])
        pkg.enc(entry, 0, 0x50)
        kind, number, checksum, _flags, off, size, _padded = struct.unpack_from('<HHHHIII', entry, 0)
        name = bytes(entry[0x40:0x50]).split(b'\0', 1)[0]
        if (kind not in REGIONS or kind <= previous_kind or number != index
                or (index < 2 and kind != (0, 100)[index])):
            raise ValueError(f'unexpected firmware payload descriptor {index}')
        expected, fixed_size = REGIONS[kind]
        if name != expected:
            raise ValueError(f'unexpected firmware payload name at {index}')
        label = expected.decode()
        if size == 0 or off < minimum_offset or off % 0x100 or off + size > len(logical):
            raise ValueError(f'payload {label} missing, misplaced or truncated')
        if fixed_size is not None and size != fixed_size:
            raise ValueError(f'invalid {label} size')
        payload = logical[off:off + size]
        if pkg.crc16(payload) != checksum:
            raise ValueError(f'{label} CRC mismatch')
        if kind == 103 and not pkg.valid_fdata(payload):
            raise ValueError(f'invalid {label} factory pack')
        minimum_offset, previous_kind = off + size, kind
        names.append(label)
    return {'model': model, 'product': product, 'bytes': len(raw),
            'sha256_check': 'package header, app and all payload CRCs verified',
            'package_crc32': f'{zlib.crc32(raw):08x}', 'payloads': names}


def check_controller(path: Path) -> dict:
    image = path.read_bytes()
    if problem := add_ctl.ctl_problem(image):
        raise ValueError(f'controller image: {problem}')
    return {'controller_bytes': len(image), 'controller_crc32': f'{zlib.crc32(image):08x}',
            'controller_header_crc': 'verified'}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('type', choices=['package', 'controller'])
    parser.add_argument('path', type=Path)
    parser.add_argument('--model', choices=sorted(PRODUCTS))
    args = parser.parse_args(argv)
    if args.type == 'package' and not args.model:
        parser.error('package requires --model')
    if args.type == 'controller' and args.model:
        parser.error('controller has no --model')
    try:
        result = (check_firmware(args.path, args.model) if args.type == 'package'
                  else check_controller(args.path))
    except (OSError, ValueError, add_ctl.PkgError, struct.error) as exc:
        print('BUILD OUTPUT INVALID: ' + str(exc), file=sys.stderr)
        return 1
    print(json.dumps(result, sort_keys=True))
    return 0


if __name__ == '__main__':
    sys.exit(main())
