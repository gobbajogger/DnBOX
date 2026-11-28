"""Compare fixed-workload pre-DMA float-to-int renderer fingerprints."""
from pathlib import Path
import struct, subprocess, sys

def read(directory):
    d=Path(directory); ram=(d/'ram.bin').read_bytes()
    symbols={}
    for line in subprocess.check_output(['readelf','-Ws',d/'bench.elf'],text=True).splitlines():
        fields=line.split()
        if len(fields)>=8 and fields[3]=='OBJECT':
            symbols[fields[7]]=int(fields[1],16)-0x01c00000
    fixture=struct.unpack_from('<9I',ram,symbols['bench_s1_only'])
    assert fixture[:8]==(0,65536,8,98304,163840,4,0,0) and fixture[8]>1
    assert struct.unpack_from('<I',ram,symbols['bench_s1_digest_count'])[0]==160
    assert 'skipped 0 cycles' in (d/'run.log').read_text()
    return fixture[8],struct.unpack_from('<160I',ram,symbols['bench_s1_digest'])

old,new=map(read,sys.argv[1:])
assert old[0]==new[0], (old[0],new[0])
delta=[i+192 for i,(a,b) in enumerate(zip(old[1],new[1])) if a!=b]
print(f'Changed pre-DMA fingerprints: {len(delta)} of 160; generated half indices: {delta}')
raise SystemExit(bool(delta))
