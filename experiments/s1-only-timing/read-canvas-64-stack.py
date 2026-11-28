from pathlib import Path
from struct import unpack_from
import sys,subprocess
d=Path(sys.argv[1])
ram=(d/'ram.bin').read_bytes()
sym={}
for l in subprocess.check_output(['readelf','-Ws',str(d/'bench.elf')],text=True).splitlines():
    t=l.split()
    if len(t)>=8 and t[0][:-1].isdigit() and t[6]!='UND':sym[t[7]]=int(t[1],16)-0x1c00000
lo,top=sym['_ustack_lo'],sym['_ustack_top']
assert 0<lo<top<len(ram)
def get(off):
    return unpack_from('<I',ram,off)[0]
assert get(lo)==0x5553544B,'stack lower guard witness missing or entirely used'
a=lo
while a<top and get(a)==0x5553544B:a+=4
print(f'stack capacity={top-lo} B; untouched marked prefix={a-lo} B; deepest observed use={top-a} B')
if 'bench_display_oversize' in sym:
    print(f"oversize={get(sym['bench_display_oversize'])}")
