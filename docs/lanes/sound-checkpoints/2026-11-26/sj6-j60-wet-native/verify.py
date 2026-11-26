from pathlib import Path
import json,struct,hashlib,re,subprocess
root=Path(__file__).resolve().parent
dig=lambda b:hashlib.sha256(b).hexdigest()
def elfsec(p):
 s=p.read_bytes()
 assert s[:6]==b'\x7fELF\x01\x01'
 off=struct.unpack_from('<I',s,32)[0]
 size,count,stri=struct.unpack_from('<HHH',s,46)
 assert size==40
 h=[struct.unpack_from('<10I',s,off+i*size) for i in range(count)]
 def sec(i):
  sh=h[i];return s[sh[4]:sh[4]+sh[5]]
 names=sec(stri)
 return {names[v[0]:names.index(0,v[0])].decode():(v,sec(i) if v[1]!=8 else b'') for i,v in enumerate(h)}
def symbol(elf,n):
 s=subprocess.check_output(['readelf','-sW',str(elf)],text=True)
 m=re.search(r'^\s*\d+:\s+([0-9a-f]+)\s+(\d+)\s+.+\s'+re.escape(n)+r'$',s,re.M)
 assert m,n
 return [int(m[1],16),int(m[2])]
output={}
for board in ('elite','pro'):
 p=root/'out'/board
 paths=[p/str(i)/'never-install.elf' for i in (0,1)]
 plain=dig(paths[0].read_bytes())
 donor=root.parent/'sj6-hp-exact-exp-fit'/f'kept-{board}-1'/'out'/board/'never-install.elf'
 assert dig(donor.read_bytes())==plain
 a,b=[elfsec(f) for f in paths]
 assert a.keys()==b.keys() and all(a[k][0]==b[k][0] for k in a)
 s0,s1=a['.text'][1],b['.text'][1]
 x,=(m.start() for m in re.finditer(b'D9RD',s0) if s0[m.start()+32:m.start()+36]==b'D9T3' and s0[m.start()+64:m.start()+68]==b'D9FT')
 assert s0[x+32:x+36]==b'D9T3' and s0[x+64:x+68]==b'D9FT'
 meta=[(x+i+20,x+i+32) for i in (0,32,64)]
 def mask(s,ranges):
  b=bytearray(s)
  for i,j in ranges:b[i:j]=bytes(j-i)
  return bytes(b)
 # Same linker-generated header build identity reused in all areas.
 for tag in ('.lab_fdata','.lab_slot3','.lab_fittab'):
  assert mask(a[tag][1],[(20,32)])==mask(b[tag][1],[(20,32)])
  assert a[tag][1][20:32]==s0[x+20:x+32] and b[tag][1][20:32]==s1[x+20:x+32]
 assert all(a[k][1]==b[k][1] for k in a if k not in {'.text','.lab_fdata','.lab_slot3','.lab_fittab'})
 q0,q1=mask(s0,meta),mask(s1,meta)
 offsets=[i for i,(u,v) in enumerate(zip(q0,q1)) if u!=v]
 i=offsets[0]
 assert offsets==list(range(i,i+4)) and i%4==0
 exact=[struct.pack('<f',v) for v in (.533294672,.2516540875455158)]
 assert [q[i:i+4] for q in (q0,q1)]==exact
 j=symbol(paths[0],'juno6_j60_process')
 assert j==symbol(paths[1],'juno6_j60_process')
 addr=i+a['.text'][0][3]
 assert j[0]<=addr<j[0]+j[1]
 syms={n:symbol(paths[0],n) for n in ('_pool_start','_pool_end','_xpool_start','_xpool_end')}
 assert all(symbol(paths[1],n)==v for n,v in syms.items())
 logs=(root/f'{board}.log').read_text()
 sizes=[list(map(int,m)) for m in re.findall(r'guarded '+board+r' XIP (\d+) free (\d+) pool usable (\d+)',logs)]
 assert len(sizes)==2 and sizes[0]==sizes[1]
 output[board]={'xip_used_free_pool_usable_above_min':sizes,'executable_sha':[dig(f.read_bytes()) for f in paths],
  'stock_exact_prior_guarded_donor':True, 'unique_literal_at':hex(addr),'literal_f32_le':[v.hex() for v in exact],
  'juno6_j60_process_start_len':[hex(j[0]),j[1]],'only_other_diffs_link_derived_ids':True,
  'raw_pool_bounds':{n:hex(v[0]) for n,v in syms.items()},'pool_min':8192}
print(json.dumps(output,indent=2))
