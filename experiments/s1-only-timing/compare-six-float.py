"""Compare per-256-block fingerprints of native stereo Synth One float words."""
from pathlib import Path
import struct, subprocess, sys
def values(path, names):
    p=Path(path); r=(p/'ram.bin').read_bytes()
    assert 'skipped 0 cycles' in (p/'run.log').read_text()
    m={}
    for line in subprocess.check_output(['readelf','-Ws',p/'bench.elf'],text=True).splitlines():
        f=line.split()
        if len(f)>=8 and f[3]=='OBJECT':m[f[7]]=int(f[1],16)-0x1c00000
    return [struct.unpack_from('<%dI'%n,r,m[k]) for k,n in names]
def read(p):
    n,e,h,w=values(p,[('six_voice_fingerprint_count',1),('six_voice_fingerprint_bad',1),('six_voice_fingerprints',320),('bench_s1_only',9)])
    assert n==(320,) and e==(0,) and w==(0,65536,12,98304,163840,6,0,0,0),(n,e,w)
    return h
a,b=map(read,sys.argv[1:])
c=[i for i,(u,v) in enumerate(zip(a,b)) if u!=v]
print(f'{len(c)} changed fingerprints (256 held spans + 64 release spans); examples={c[:12]}')
assert not c
