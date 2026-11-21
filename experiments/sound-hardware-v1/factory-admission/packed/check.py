#!/usr/bin/env python3
"""Compare C lookup and every decoded word to the source-bank order and raw command stream."""
import argparse
import ctypes as ct
import json
from pathlib import Path
import struct
import subprocess
import tempfile

from generate import BANKS, build

HERE = Path(__file__).resolve().parent


def main():
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument('commands', type=Path)
    args = cli.parse_args()
    with tempfile.TemporaryDirectory() as temp:
        out = Path(temp)
        with (out / 's1_factory_bank.inc').open('w') as f:
            build(args.commands / 'ORIGINAL-PRESET-COMMANDS.json', f)
        lib = out / 'bank.so'
        subprocess.run(['cc', '-O2', '-std=c11', '-fPIC', '-shared', '-Wall', '-Wextra', '-Werror',
                        '-I', out, HERE / 'bank.c', '-o', lib], check=True)
        api = ct.CDLL(str(lib))
        api.s1_factory_lookup.argtypes = [ct.c_char_p, ct.c_uint]
        api.s1_factory_name.argtypes = [ct.c_uint]
        api.s1_factory_name.restype = ct.c_char_p
        api.s1_factory_read.argtypes = [ct.c_uint, ct.c_uint, ct.POINTER(ct.c_uint), ct.POINTER(ct.c_float)]
        parameter, value = ct.c_uint(0xfeed), ct.c_float(-3.25)
        assert api.s1_factory_lookup(None, 0) == api.s1_factory_lookup(b'unknown', 0) == -1
        assert api.s1_factory_lookup(b'Bonus', 39) == api.s1_factory_lookup(b'Bonus', 134) == -1
        assert api.s1_factory_name(694) is None
        for program, command in [(694, 0), (0, 144), (2**32-1, 0)]:
            assert not api.s1_factory_read(program, command, ct.byref(parameter), ct.byref(value))
        assert not api.s1_factory_read(0, 0, None, ct.byref(value))
        assert not api.s1_factory_read(0, 0, ct.byref(parameter), None)
        assert (parameter.value, value.value) == (0xfeed, -3.25)
        seen = set()
        count = 0
        with (args.commands / 'preset-commands.bin').open('rb') as commands:
            assert struct.unpack('<II', commands.read(8)) == (0x53315043, 694)
            banks = json.loads((BANKS / 'MANIFEST.json').read_bytes())['files']
            for entry in sorted(banks, key=lambda e: e['name']):
                bank = BANKS / entry['name']
                for p in json.loads(bank.read_bytes()):
                    idx = api.s1_factory_lookup(bank.stem.encode(), p['position'])
                    assert idx >= 0 and idx not in seen
                    seen.add(idx)
                    assert api.s1_factory_name(idx).decode() == p['name']
                    assert struct.unpack('<I', commands.read(4))[0] == 144
                    for i in range(144):
                        assert api.s1_factory_read(idx, i, ct.byref(parameter), ct.byref(value))
                        assert commands.read(8) == struct.pack('<If', parameter.value, value.value)
                        count += 1
            assert not commands.read(1) and len(seen) == 694
        print(f'{len(seen)} unique addresses, {count} exact ordered words; invalid selections preserve outputs')


if __name__ == '__main__':
    main()
