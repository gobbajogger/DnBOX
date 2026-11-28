"""Read strict, native six-dry Synth One plus two-kit timing and validate its scope."""
import sys, struct, subprocess, json
from pathlib import Path
d=Path(sys.argv[1])
tab={}
for l in subprocess.check_output(['readelf','-Ws',d/'bench.elf'], text=True).splitlines():
    t=l.split()
    if len(t)>=8 and t[6]!='UND' and t[3] in ('FUNC','OBJECT'):
        tab[t[7]]=int(t[1],16)-0x1c00000
for name in ('engine_superj6_factory_load','engine_superj6_edit','juno6_delay24_process'):
    assert name not in tab,name
b=(d/'ram.bin').read_bytes()
r=lambda n,k:struct.unpack_from('<'+str(k)+'I',b,tab[n])
guard=r('bench_s1_only',9); held=r('bench_s1_held',4); dbg=r('x0x_dbg',13); xruns=r('audio_xruns',1)[0]
print(json.dumps(dict(fixture=guard,held_halves=held[0],held_peak_us=held[1]/24,held_over_target=held[2],held_over_physical=held[3],max_us=dbg[2],halves=dbg[1],late=dbg[5],xruns=xruns)))
assert guard[:8]==(0,65536,12,98304,163840,6,0,0) and guard[8]==0
assert held[0]==128 and 'skipped 0 cycles' in (d/'run.log').read_text()
assert r("bench_loaded_hits",2)==(132,132)
