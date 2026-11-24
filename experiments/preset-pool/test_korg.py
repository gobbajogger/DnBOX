#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Use original compiled factory functions as independent source oracle.
Compare exact full program structs, all native parameter words, routing,
legacy names and rejection. All emitted banks are local test inputs."""
import ctypes as C
import os, subprocess, sys, tempfile
from pathlib import Path
from encode import encode
D=Path(__file__).resolve().parent
SRC=Path(sys.argv[1]).resolve()
P=C.c_void_p; U=C.c_uint
class Chicago(C.Structure):_fields_=[('macro',C.c_float*22),('target',C.c_float*29),('rev',C.c_uint32)]
class Miami(C.Structure):_fields_=[('macro',C.c_float*18),('target',C.c_float*24),('rev',C.c_uint32)]
def build(out,alt,variant):
 src=variant if alt else SRC
 subprocess.run([os.environ.get('CC','cc'),'-std=c11','-O2','-fno-fast-math','-ffp-contract=off','-shared','-fPIC','-Wl,-Bsymbolic','-I'+str(SRC),'-I'+str(D),*[str(src/f'{n}_program.c') for n in ('chicago','miami')],*([str(D/'korg_factory.c'),str(D/'dnbank.c')] if alt else []),'-lm','-o',str(out)],check=True)
 return C.CDLL(str(out))
def word(v):return int.from_bytes(bytes(v),'little')
with tempfile.TemporaryDirectory() as tmp:
 variant=Path(tmp)/'variant'
 subprocess.run([sys.executable,str(D/'make_variant.py'),str(SRC),str(variant)],check=True)
 old=build(Path(tmp)/'old.so',False,variant); new=build(Path(tmp)/'new.so',True,variant)
 new.dnbank_open.argtypes=[P,P,C.c_size_t];new.dnbank_open_mapped.argtypes=[P,P,C.c_size_t,P,U,C.c_size_t];new.dn_korg_bind.argtypes=[U,P]
 keep=[]; count=words=native=0
 for e,(name,N,T,native_n) in enumerate([('chicago',81,Chicago,117),('miami',30,Miami,109)]):
  for lib in (old,new):
   getattr(lib,f'original_{name}_program_init').argtypes=[P,U];getattr(lib,f'original_{name}_program_name').argtypes=[U];getattr(lib,f'original_{name}_program_name').restype=C.c_char_p;getattr(lib,f'original_{name}_program_native').argtypes=[P,U,P]
  a=getattr(old,f'original_{name}_program_init');b=getattr(new,f'original_{name}_program_init');an=getattr(old,f'original_{name}_program_name');bn=getattr(new,f'original_{name}_program_name');av=getattr(old,f'original_{name}_program_native');bv=getattr(new,f'original_{name}_program_native')
  rows=[]
  for i in range(N):
   t=T();assert a(C.byref(t),i)
   raw=bytes(t.macro); rows.append(((i+1).to_bytes(8,'little'),an(i).decode(),[(j,int.from_bytes(raw[j*4:j*4+4],'little')) for j in range(len(t.macro))]))
  blob=encode(e,bytes([e+1])*8,name.title(),rows);(variant/f'{name}.dnl2').write_bytes(blob)
  # Decode from non-contiguous physical sectors to exercise the same mapping as the pool.
  sectors=(len(blob)+4095)//4096; order=list(reversed(range(sectors))); image=bytearray(b'\xff'*(4096*sectors))
  for i,s in enumerate(order): image[s*4096:s*4096+len(blob[i*4096:(i+1)*4096])]=blob[i*4096:(i+1)*4096]
  buf=C.create_string_buffer(bytes(image)); map_=(C.c_uint8*sectors)(*order); view=C.create_string_buffer(128); keep.extend([buf,map_,view]);assert new.dnbank_open_mapped(view,buf,len(image),map_,sectors,len(blob))
  blank=T();before=bytes(blank);assert not b(C.byref(blank),0) and bytes(blank)==before
  assert new.dn_korg_bind(e,view)
  for i in range(N):
   x,y=T(),T(); assert a(C.byref(x),i) and b(C.byref(y),i); assert bytes(x)==bytes(y),(name,i)
   assert an(i)==bn(i),(name,i)
   for j in range(native_n):
    v,w=C.c_float(),C.c_float();assert av(C.byref(x),j,C.byref(v)) and bv(C.byref(y),j,C.byref(w));assert bytes(v)==bytes(w),(name,i,j);native+=1
   if e==0:
    # Zero-filled oversized buffers: compare complete returned routing struct.
    r,s=C.create_string_buffer(64),C.create_string_buffer(64);assert old.original_chicago_program_routing(C.byref(x),r) and new.original_chicago_program_routing(C.byref(y),s);assert r.raw==s.raw
   count+=1;words+=len(x.macro)
  assert new.dn_korg_bind(e,None);assert not b(C.byref(blank),0) and bytes(blank)==before;assert bn(0) is None
  print(name,N,len(blob),'bytes',sectors,'sectors',flush=True)
 print('PASS:',count,'exact preset structs/names;',words,'macro words;',native,'native words; 81 Chicago routing snapshots; unbound rejection')
