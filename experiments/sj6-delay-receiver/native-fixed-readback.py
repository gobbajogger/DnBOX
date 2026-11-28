from pathlib import Path
import subprocess,struct,re,sys,json
d=Path(sys.argv[1]);elf=d/'bench.elf';ram=(d/'ram.bin').read_bytes()
sy={}
for line in subprocess.check_output(['readelf','-Ws',elf],text=True).splitlines():
    r=line.split()
    if len(r)>=8 and r[3]=='OBJECT' and r[7] in ('bench_fixed','x0x_dbg','audio_xruns','lab_delay_probe','lab_s1_probe'):
        sy[r[7]]=int(r[1],16)-0x1c00000
row=struct.unpack_from('<10I',ram,sy['bench_fixed'])
dbg=struct.unpack_from('<13I',ram,sy['x0x_dbg'])
xruns,=struct.unpack_from('<I',ram,sy['audio_xruns'])
digest=0
notes=(48,48,52,55,59)
for sample in (98304,163840):
    for i,n in enumerate(notes):
        v=0 if sample==163840 else (88 if i else 86)
        x=(bool(i)<<24)|(n<<16)|(v<<8)|((sample>>8)&255)
        digest=((digest*16777619)&0xffffffff)^x
summary=dict(fixture=row,expected_hash=digest,late=dbg[5],xruns=xruns,max_us=dbg[2],halves=dbg[1])
print(json.dumps(summary),flush=True)
assert row[:8]==(0,10,digest,65536,98304,163840,4,0)
assert 'skipped 0 cycles' in (d/'run.log').read_text()
