#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Packed Synth One factory programs (tools/pack_synthone_factory.py, firmware/src/dsp/s1pack).

Packs the 12 authenticated banks, then the C reader decodes every program's
name and all 144 ordered physical writes from an unaligned-in-file base and
compares them bit for bit. Also: the pack fits the factory synth data budget
for Synth One (15 sectors), the reader keeps zero static RAM, and corrupt or
truncated packs are rejected."""
import json, os, shutil, struct, subprocess, sys, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
import importlib.util
spec = importlib.util.spec_from_file_location('pack', ROOT / 'tools/pack_synthone_factory.py')
pack = importlib.util.module_from_spec(spec); spec.loader.exec_module(pack)

BUDGET = 15 * 0x1000          # Synth One share of FL_FDATA: 0xF000 bytes


def main():
    work = Path(tempfile.mkdtemp(prefix='s1pack-'))
    try:
        subprocess.run([sys.executable, str(ROOT / 'tools/compile-synthone-factory.py'), '--out', str(work)],
                       check=True, stdout=subprocess.DEVNULL)
        programs, banks, order = pack.load_programs(work / 'ORIGINAL-PRESET-COMMANDS.json')
        blob, sizes = pack.pack(programs, banks, order)
        again, _ = pack.pack(programs, banks, order)
        assert blob == again, 'pack is not deterministic'
        (work / 's1.pack').write_bytes(blob)
        with (work / 'expect.bin').open('wb') as f:
            for name, cmds in programs:
                nb = name.encode()
                f.write(bytes([len(nb)]) + nb + struct.pack('<I', len(cmds)))
                for i, v in cmds:
                    f.write(struct.pack('<If', i, v))
        exe = work / 's1pack_roundtrip'
        cc = os.environ.get('CC', 'cc')
        base = [cc, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', '-pedantic',
                str(ROOT / 'tests/host/s1pack_roundtrip_test.c'), str(ROOT / 'firmware/src/dsp/s1pack/s1pack.c'),
                '-o', str(exe)]
        san = subprocess.run(base[:1] + ['-fsanitize=address,undefined'] + base[1:], capture_output=True)
        if san.returncode:
            subprocess.run(base, check=True)
        out = subprocess.run([exe, work / 's1.pack', work / 'expect.bin'], capture_output=True, text=True)
        print(out.stdout.strip())
        ok = out.returncode == 0 and out.stdout.strip().endswith('PASS')
        # negatives: a flipped stream bit changes the CRC; a bad magic or version is refused
        bad = bytearray(blob); bad[len(bad) // 2] ^= 0x10
        (work / 'bad.pack').write_bytes(bad)
        neg = subprocess.run([exe, work / 'bad.pack', work / 'expect.bin'], capture_output=True, text=True)
        ok &= neg.returncode != 0 and 'FAIL header' in neg.stdout
        for at, val in ((0, 0x58), (4, 2)):
            bad = bytearray(blob); bad[at] = val
            (work / 'bad.pack').write_bytes(bad)
            neg = subprocess.run([exe, work / 'bad.pack', work / 'expect.bin'], capture_output=True, text=True)
            ok &= neg.returncode != 0 and 'programs=0' in neg.stdout
        # the reader has no static storage
        obj = work / 's1pack.o'
        subprocess.run([cc, '-std=c11', '-Os', '-c', str(ROOT / 'firmware/src/dsp/s1pack/s1pack.c'), '-o', str(obj)], check=True)
        syms = subprocess.run(['nm', obj], capture_output=True, text=True, check=True).stdout
        statics = [l for l in syms.splitlines() if l.split()[-2:-1] and l.split()[-2] in 'bBdDcC']
        ok &= not statics
        print('s1pack sizes ' + json.dumps(sizes))
        print(f's1pack: bytes={len(blob)} budget={BUDGET} sectors={(len(blob) + 0xFFF) // 0x1000} '
              f'static_ram={len(statics)}')
        ok &= len(blob) <= BUDGET and sizes['total'] == len(blob)
        print('PASS' if ok else 'FAIL')
        return 0 if ok else 1
    finally:
        shutil.rmtree(work, ignore_errors=True)


if __name__ == '__main__':
    sys.exit(main())
