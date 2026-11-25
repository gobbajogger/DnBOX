"""Real PI32 linker test of the lower bound for relocating fixed assets into APP."""
import argparse
import contextlib
import importlib.util
import io
import json
from pathlib import Path
import re
import subprocess
import tempfile

from link_residency import factory_data,codec

def arr(path):
    s=path.read_text()
    p=s.split('{',1)[1].rsplit('}',1)[0]
    return bytes(int(c.strip(),0)for c in p.split(',')if c.strip())

def report(bits,tc,temp):
    need={'909-packed':50961,'Chicago-A':6099,'Chicago-E':15629}
    assert {n:len(b)for n,b in bits.items()}==need
    asm=[]
    total=sum(need.values())
    for i,(_,b) in enumerate(bits.items()):
        name=f'a{i}'
        (temp/(name+'.bin')).write_bytes(b)
        asm.append(f'.section .{name},"a"\n'
                   +(f'.global _asset_start\n_asset_start:\n' if i==0 else '')
                   +f'.incbin "{name}.bin"\n')
    (temp/'app.S').write_text(''.join(asm))
    subprocess.run([str(tc/'common/bin/clang'),'-target','pi32v2','-mcpu=r3',
        '-c','app.S','-o','app.o'],cwd=temp,check=True)
    for name,free in [('elite',352),('pro',360),('control',total)]:
        # Bind to the real physical end of the measured APP window; used code
        # excluded deliberately because presence is represented by its measured
        # headroom, not a rebuilt firmware image.
        top=0x02000120+0x86ee0
        (temp/'app.ld').write_text(
            f'MEMORY {{ A (r) : ORIGIN = {top-free:#x}, LENGTH = {free} }}\n'
            'SECTIONS { .fixed : { *(.a0) *(.a1) *(.a2) } > A }\n')
        p=subprocess.run([str(tc/'pi32v2/bin/ld'),'-T','app.ld','-Map','app.map','-e','_asset_start',
                           'app.o','-o','never-install.elf'],cwd=temp,capture_output=True,text=True)
        excess=total-free
        if excess:
            assert p.returncode!=0 and "will not fit in region" in p.stderr,p.stderr
            mp=(temp/'app.map').read_text()
            m=re.search(r'^\.fixed\s+0x([a-fA-F\d]+)\s+0x([a-fA-F\d]+)',mp,re.M)
            assert m and int(m[1],16)==top-free and int(m[2],16)==total,mp[-1500:]
        else:
            assert p.returncode==0,p.stderr
            out=subprocess.check_output(['readelf','-SW','never-install.elf'],cwd=temp,text=True)
            assert int(re.search(r'\.fixed\s+PROGBITS\s+\w+\s+\w+\s+(\w+)',out)[1],16)==total
        print(json.dumps(dict(label=name,known_APP_slack_B=free,
              exact_fixed_subset_B=total,actual_link_overflow_B=excess)))

def main():
    p=argparse.ArgumentParser()
    for name in ('firmware','codec','toolchain','filter_generator'):
        p.add_argument('--'+name.replace('_','-'),required=True,type=Path)
    a=p.parse_args()
    for k,v in vars(a).items():setattr(a,k,v.resolve())
    _br,d9=factory_data(a,codec(a.codec))
    source=a.firmware/'experiments/original_synth/chicago_packed_private.h'
    with tempfile.TemporaryDirectory(prefix='dnbox-private-relocate-') as d:
        temp=Path(d)
        subprocess.run(['python3',str(a.filter_generator),str(source),'--fast32','--emit-test',str(temp)],
                       check=True,stdout=subprocess.DEVNULL)
        report({'909-packed':d9,'Chicago-A':arr(temp/'p_a.h'),'Chicago-E':arr(temp/'p_e.h')},
               a.toolchain,temp)

if __name__=='__main__':main()
