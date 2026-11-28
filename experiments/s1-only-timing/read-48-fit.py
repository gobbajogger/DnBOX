"""Verify the actual linked raster size and absence of a clipped raster request."""
from pathlib import Path
from struct import unpack_from
from subprocess import check_output
import sys

run = Path(sys.argv[1])
assert 'skipped 0 cycles' in (run/'run.log').read_text()
symbols = {}
for line in check_output(['readelf','-Ws',run/'bench.elf'],text=True).splitlines():
    f=line.split()
    if len(f)>7 and f[3]=='OBJECT':
        symbols[f[7]]=int(f[1],16)-0x1c00000,int(f[2],0)
ram=(run/'ram.bin').read_bytes()
index,size=symbols['bench_display_oversize']
assert size==4 and symbols['cv_px'][1]==48*240*2
oversize=unpack_from('<I',ram,index)[0]
print(f'48 rows / {symbols["cv_px"][1]} B; oversized requests: {oversize}')
assert oversize==0
