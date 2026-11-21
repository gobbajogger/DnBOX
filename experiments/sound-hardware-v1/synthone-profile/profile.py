#!/usr/bin/env python3
"""Sample active callback instructions on Linux x86-64 with original IEEE arithmetic."""
import argparse
import bisect
import collections
import hashlib
import json
import os
import re
import struct
import subprocess
import tarfile
from pathlib import Path


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('build', type=Path)
    ap.add_argument('callback_packet', type=Path)
    ap.add_argument('out', type=Path)
    args = ap.parse_args()
    base, out = args.build.resolve(), args.out.resolve()
    out.mkdir(exist_ok=False)
    with tarfile.open(args.callback_packet, 'r:gz') as ar:
        source = ar.extractfile('bench.cpp').read()
        rows = [json.loads(line) for line in ar.extractfile('full/original.jsonl')]
    contract_input = hashlib.sha256(source).hexdigest()
    if contract_input != '6cc3407fffc13a8659967b4c92a53988f7d8d6d44dffe79110eb9e431da22170':
        raise ValueError('unexpected callback fixture')
    slowest = sorted(rows, key=lambda r: (-r['cpu_misses'], -r['max_cpu_ns'], r['preset']))[:8]
    cases = [r['preset'] for r in slowest] * 8
    stream = (base / 'commands/preset-commands.bin').read_bytes()
    stride = 4 + 144 * 8
    if struct.unpack_from('<II', stream) != (0x53315043, 694) or len(stream) != 8 + 694 * stride:
        raise ValueError('unexpected command layout')
    # Keep the first eight originals in CPU-miss order and repeat for sampling power.
    # The callback bench expects the full archive count and reads only the requested prefix.
    (out / 'commands.bin').write_bytes(stream[:8] + b''.join(stream[8+i*stride:8+(i+1)*stride] for i in cases))
    source = source.decode()
    for old,new in [
        ('#include <time.h>', '#include <time.h>\nextern "C" { void profile_setup(); void profile_collect(int); void profile_save(); }'),
        ('    Fixture fixture(argv[1]);','    profile_setup();\n    Fixture fixture(argv[1]);'),
        ('                NoHeap guard;', '                profile_collect(1);\n                NoHeap guard;'),
        ('                CHECK(synth.process(128, 0, left, right));',
         '                CHECK(synth.process(128, 0, left, right));\n                profile_collect(0);'),
        ('    return total_wall_misses', '    profile_save();\n    return total_wall_misses')
    ]:
        if source.count(old) != 1:
            raise ValueError(old)
        source = source.replace(old, new)
    (out / 'profile.cpp').write_text(source)
    commands = json.loads((base / 'factory-build-commands.json').read_text())
    c = next(line.copy() for line in commands if '-c' in line and line[line.index('-c')+1].endswith('/preset_audio.cpp'))
    prior = c[c.index('-o')+1]
    for flag, file in (('-c','profile.cpp'),('-o','profile.o'),('-MF','profile.d')):
        c[c.index(flag)+1] = str(out / file)
    subprocess.run(c, check=True)
    subprocess.run(['cc','-O2','-c',Path(__file__).with_name('sample.c'),'-o',out / 'sample.o'],check=True)
    c = [str(out / 'profile.o') if v == prior else v for v in commands[-1]]
    c[1:1] = ['-no-pie', str(out / 'sample.o')]
    c[c.index('-o')+1] = str(out / 'profile')
    subprocess.run(c,check=True)
    with (out / 'log').open('w') as log:
        rc = subprocess.run([out / 'profile',base / 'factory-inputs/public-morphs.f32',base / 'factory-inputs/compact.bin',
                             out / 'commands.bin','trace.csv','rows.jsonl',str(len(cases)),'0'],
                            cwd=out,stdout=log,stderr=subprocess.STDOUT).returncode
    if rc not in (0,1):
        raise ValueError(f'profile run returned {rc}')
    got = [json.loads(line) for line in (out / 'rows.jsonl').read_text().splitlines()]
    if len(got) != len(cases) or any(r['nonfinite_words'] or r['diagnostic_fnv64'] != rows[i]['diagnostic_fnv64']
                                     for i,r in zip(cases,got)):
        raise ValueError('audio differs from original callback fixture')
    modules = []
    for line in (out / 'mappings').read_text().splitlines():
        match = re.match(r'([0-9a-f]+)-([0-9a-f]+) r-x. ([0-9a-f]+) \S+\s+\d+\s+(/.*)$',line)
        if not match:
            continue
        lo, hi, offset = (int(n,16) for n in match.groups()[:3])
        path = Path(match[4])
        table = []
        for dynamic in (False,True):
            result = subprocess.run(['nm','-n','-S','-C',*(['-D'] if dynamic else []),path],
                                    capture_output=True,text=True)
            for entry in result.stdout.splitlines():
                m=re.match(r'^([0-9a-f]+) ([0-9a-f]+) [TWtwIi] (.+)$',entry)
                if m:
                    start,size,name = m.groups()
                    table.append((int(start,16),int(size,16),name))
            if table:
                break
        table.sort()
        modules.append((lo,hi,0 if path == out/'profile' else lo-offset,path.name,table,[e[0] for e in table]))
    counts = collections.Counter()
    for (pc,) in struct.iter_unpack('=Q',(out / 'sample-pcs.bin').read_bytes()):
        label = 'unmapped'
        for lo,hi,slide,module,table,starts in modules:
            if lo <= pc < hi:
                x = pc-slide
                i = bisect.bisect_right(starts,x)-1
                label = module + ' / unresolved'
                if i >= 0 and x < table[i][0]+table[i][1]:
                    label = module + ' / ' + table[i][2].split('@@')[0]
                break
        counts[label]+=1
    outdata=dict(programs=[r['preset'] for r in slowest],repetitions=8,callbacks=len(cases)*750,
                 samples=sum(counts.values()),pcm_checks=len(got),fixture_sha256=contract_input,
                 log=(out/'log').read_text(),functions=counts.most_common())
    (out/'summary.json').write_text(json.dumps(outdata,indent=2)+'\n')


if __name__ == '__main__':
    main()
