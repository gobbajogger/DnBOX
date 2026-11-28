"""Read actual successful legacy owner draws from diagnostic native RAM."""
from pathlib import Path
from struct import unpack_from
from subprocess import check_output
import sys
p=Path(sys.argv[1])
assert 'skipped 0 cycles' in (p/'run.log').read_text()
symbols={}
for l in check_output(['readelf','-Ws',p/'bench.elf'],text=True).splitlines():
    a=l.split()
    if len(a)>=8 and a[3]=='OBJECT':
        symbols[a[7]]=(int(a[1],16)-0x1c00000,int(a[2],0))
raw=(p/'ram.bin').read_bytes()
off, n=symbols['bench_legacy_matrix']
assert n==8
counts=unpack_from('<II',raw,off)
assert all(counts),counts
a,n=symbols['bench_display_oversize']
assert n==4 and unpack_from('<I',raw,a)==(0,)
assert symbols['cv_px'][1]==48*240*2
print(f'Chicago frames {counts[0]}, Miami frames {counts[1]}; 48-row raster; zero oversized requests')
