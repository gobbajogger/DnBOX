#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Standalone regression: no private source bank required."""
import ctypes as C
import os
from pathlib import Path
import random
import struct
import subprocess
import tempfile
import zlib
from encode import encode, MAX_FIELDS

HERE = Path(__file__).resolve().parent
P, U = C.c_void_p, C.c_uint
EMIT = C.CFUNCTYPE(None, P, U, C.c_uint32)


def main():
    with tempfile.TemporaryDirectory() as tmp:
        so = Path(tmp) / 'reader.so'
        subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-Os', '-shared', '-fPIC', '-fsanitize=undefined', '-fno-sanitize-recover=all', str(HERE/'dnbank.c'), '-o', str(so)], check=True)
        lib = C.CDLL(str(so))
        lib.dnbank_open.argtypes = [P, P, C.c_size_t]
        lib.dnbank_open_mapped.argtypes = [P, P, C.c_size_t, P, U, C.c_size_t]
        lib.dnbank_read.argtypes = [P, U, P, EMIT, P]
        lib.dnbank_validate.argtypes = [P]
        lib.dnbank_preset_id.argtypes = [P, U, P]
        rng = random.Random(20261124)
        for engine, limit in enumerate(MAX_FIELDS):
            values = [0, 0x80000000, 0x00000001, 0x7f7fffff, 0xff7fffff, 0x3f800000] + list(range(0x3e000000, 0x3e000080))
            rows = [(i.to_bytes(8, 'little'), f'Patch {i}', [(rng.randrange(limit), rng.choice(values)) for _ in range(1 + i % 100)]) for i in range(80)]
            blob = encode(engine, bytes((engine,))*8, 'Test', rows)
            b = C.create_string_buffer(128)
            name = C.create_string_buffer(48)
            pid = C.create_string_buffer(8)
            data = C.create_string_buffer(blob)
            assert lib.dnbank_open(b, data, len(blob)) and lib.dnbank_validate(b)
            got = []
            cb = EMIT(lambda _, i, w: got.append((i, w)))
            for i, row in enumerate(rows):
                got.clear()
                assert lib.dnbank_read(b, i, name, cb, None) == len(row[2])
                assert got == row[2] and name.value.decode() == row[1]
                assert lib.dnbank_preset_id(b, i, pid) and pid.raw == row[0]
            # Cross physical sector boundaries and reverse their locations.
            n = (len(blob)+4095)//4096
            physical = bytearray(b'\xff'*(n*4096))
            order = list(reversed(range(n)))
            for i, loc in enumerate(order):
                block = blob[4096*i:4096*(i+1)]
                physical[4096*loc:4096*loc+len(block)] = block
            data = C.create_string_buffer(bytes(physical))
            mapping = (C.c_uint8*n)(*order)
            assert lib.dnbank_open_mapped(b, data, len(physical), mapping, n, len(blob)) and lib.dnbank_validate(b)
            # Corrupt every section offset, then recompute CRC: header validation must still reject.
            for off in range(32, 46, 2):
                bad = bytearray(blob)
                struct.pack_into('<H', bad, off, 65535)
                struct.pack_into('<I', bad, 8, zlib.crc32(bad[:8]+bad[12:]))
                d = C.create_string_buffer(bytes(bad))
                assert not lib.dnbank_open(b, d, len(bad))
            # Duplicate identity with a good CRC must fail complete validation.
            bad = bytearray(blob)
            off = struct.unpack_from('<H', bad, 42)[0]
            a = off + 2*len(rows)
            second = a + struct.unpack_from('<H', bad, off+2)[0]
            bad[second:second+8] = bad[a:a+8]
            struct.pack_into('<I', bad, 8, zlib.crc32(bad[:8]+bad[12:]))
            d = C.create_string_buffer(bytes(bad))
            assert lib.dnbank_open(b, d, len(bad)) and not lib.dnbank_validate(b)
            subset = rows[::3]
            data = C.create_string_buffer(encode(engine, bytes((engine,))*8, 'Test', subset))
            assert lib.dnbank_open(b, data, len(data)-1) and lib.dnbank_validate(b)
            for i, row in enumerate(subset):
                assert lib.dnbank_preset_id(b, i, pid) and pid.raw == row[0]
        print('PASS: 4 schemas, 320 synthetic presets, exact ordered words, remapped sectors, corrupt offsets, duplicate ID rejection, stable subset identities')


if __name__ == '__main__':
    main()
