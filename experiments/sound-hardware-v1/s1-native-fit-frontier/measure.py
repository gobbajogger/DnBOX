from pathlib import Path
from hashlib import sha256
import re, subprocess, json
d=Path(__file__).resolve().parent; r=d.parent
t=r/'prereqs/jieli-linux-toolchains-20260730.1/pi32v2/bin'
n=r/'sj6-normal-shared-state'
models={}
for board in ('elite','pro'):
 p=n/'tree/joint-diagnostic-later'/board
 elf=p/'never-install.elf'; h=sha256(elf.read_bytes()).hexdigest()
 log=(n/(board+'.log')).read_text()
 m=re.search(r'^guarded .*?XIP (\d+) free (\d+) pool usable (\d+).* ELF ([0-9a-f]{64})$',log,re.M)
 assert m and h==m[4]
 s=(p/'lab.map').read_text()
 sec={k:tuple(int(x,16) for x in re.search(r'^\.'+k+r'\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)',s,re.M).groups()) for k in ('bss','data','pool','xpool')}
 ram=98304-(sec['bss'][0]+sec['bss'][1]-sec['data'][0])
 assert sec['pool'][1]+int(m[3])+8192==0x54000 and sec['xpool'][1]==0x2400
 models[board]=dict(elf_sha256=h,xip_free=int(m[2]),pool_beyond_gate=int(m[3])-512,bss_free=ram,sections=sec)
assert models['elite']['sections']==models['pro']['sections']
used=models['elite']['sections']['pool'][1]
space=models['elite']['pool_beyond_gate']
budget=0x54000
txt='''MEMORY { POOL (rw) : ORIGIN = 0x01c20000, LENGTH = 0x54000 }
SECTIONS { .pool (NOLOAD) : { *(.pool) . = ALIGN(4); } > POOL
 ASSERT(SIZEOF(.pool) <= LENGTH(POOL)-8704, "floor plus reserve guard") }'''
(d/'frontier.ld').write_text(txt)
out={}
for name,count in dict(edge=space,one_word_over=space+4,minimum_25bit_one=2067290,minimum_25bit_two=4134580).items():
 exact=(used+count+3)&~3
 src=d/(name+'.S'); ob=d/(name+'.o'); dest=d/(name+'.elf')
 src.write_text('.section .pool,"aw",@nobits\n.space %d\n.space %d\n'%(used,count))
 subprocess.run([t/'clang','-target','pi32v2','-mcpu=r3','-c',src,'-o',ob],check=True)
 p=subprocess.run([t/'ld','-T',d/'frontier.ld',ob,'-o',dest],capture_output=True,text=True)
 assert (p.returncode==0)==(name=='edge'),p.stderr
 out[name]=dict(native_rc=p.returncode,extra=count,region_over=max(0,exact-budget),guard_over=max(0,exact-budget+8704),linker=p.stderr.replace(str(d)+'/','').strip())
print(json.dumps(dict(scope="native PI32 allocation-only model matching latest measured paired guarded maps; no kernel linked",models=models,cases=out),indent=2))
