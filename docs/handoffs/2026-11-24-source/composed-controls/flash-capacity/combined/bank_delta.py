"""Exact one-preset edit plus deletion without rewriting a whole bank.

Prototype format only. The base remains live, and full-bank replacement may
still need full staging. This test changes no flash or source preset archive.
"""
import argparse
import hashlib
import json
from pathlib import Path
import runpy
import struct
import sys
import types
import zlib

HEADER=struct.Struct('<4s8s16sBB')
EDIT=struct.Struct('<8sBII')
DROP=struct.Struct('<8s')
SUM=struct.Struct('<I')
SECTOR=4096

def seal(bid,base,edits,drops):
    assert len(bid)==8 and len(edits)<=255 and len(drops)<=255
    obj=HEADER.pack(b'BDE1',bid,hashlib.sha256(base).digest()[:16],len(edits),len(drops))
    obj+=b''.join(EDIT.pack(*e) for e in edits)
    obj+=b''.join(DROP.pack(d) for d in drops)
    return obj+SUM.pack(zlib.crc32(obj))

def read(bid,base,rows,obj):
    assert len(obj)>=HEADER.size+SUM.size
    m,which,digest,ne,nd=HEADER.unpack_from(obj)
    assert m==b'BDE1' and bid==which and digest==hashlib.sha256(base).digest()[:16]
    assert len(obj)==HEADER.size+ne*EDIT.size+nd*DROP.size+SUM.size
    assert zlib.crc32(obj[:-4])==SUM.unpack_from(obj,len(obj)-4)[0]
    rec={i:(p,n,list(fs))for i,p,n,fs in rows}
    assert len(rec)==len(rows)
    cur=HEADER.size
    for _ in range(ne):
        i,f,w,v=EDIT.unpack_from(obj,cur);cur+=EDIT.size
        fs=rec[i][2]
        loc=[k for k,(g,z) in enumerate(fs) if g==f and z==w]
        assert len(loc)==1
        fs[loc[0]]=(f,v)
    for _ in range(nd):
        i,=DROP.unpack_from(obj,cur);cur+=DROP.size
        del rec[i]
    return [(i,p,n,tuple(fs)) for i,(p,n,fs) in rec.items()]

def check(model,replaced,banks,target,old,new,decode,fresh):
    bid,rows=banks[target]
    eng,_=target
    # Same previously validated case the dense-format experiment re-encoded.
    assert decode(eng,bid,old)==rows and decode(eng,bid,new)==fresh
    a,_,_,f0=rows[0]
    assert a==fresh[0][0] and len(f0)==len(fresh[0][3])
    f,w=f0[0];g,v=fresh[0][3][0]
    assert f==g and v!=w
    obj=seal(bid,old,[(a,f,w,v)],[rows[1][0]])
    assert read(bid,old,rows,obj)==fresh
    # Separate directory publishes payload only after full validation.
    # Exhaust every torn prefix of the delta and its 24 B directory commit.
    entry=struct.pack('<I16s',1,hashlib.sha256(obj).digest()[:16])
    entry+=SUM.pack(zlib.crc32(entry))
    blank=lambda n:b'\xff'*n
    for n in range(len(obj)+1):
        for d in range(len(entry)+1):
            data=obj[:n]+blank(len(obj)-n)
            directory=entry[:d]+blank(len(entry)-d)
            valid=False
            try:
                seq,tag=struct.unpack_from('<I16s',directory)
                valid=(seq==1 and zlib.crc32(directory[:-4])==SUM.unpack_from(directory,20)[0]
                       and hashlib.sha256(data).digest()[:16]==tag)
                result=read(bid,old,rows,data) if valid else rows
            except (ValueError,struct.error,AssertionError):
                result=rows
            assert result==fresh if data==obj and directory==entry else result==rows
    for k,entry_bytes in replaced.items():
        if k!=target: assert decode(k[0],banks[k][0],entry_bytes)==banks[k][1]
    sectors=lambda n:(n+SECTOR-1)//SECTOR
    print(json.dumps(dict(records=sum(len(v[1]) for v in banks.values()),
       original_replacement_bytes=len(new),exact_delta_bytes=len(obj),
       normal_append_sectors=sectors(len(new)),small_edit_sectors=sectors(len(obj)),
       directory_record_bytes=len(entry),prefix_cases=(len(obj)+1)*(len(entry)+1),
       no_change_to_unedited_banks=True,offline=True),indent=2))

def main():
    p=argparse.ArgumentParser()
    p.add_argument('offline_dir',type=Path)
    p.add_argument('source_helpers',type=Path)
    p.add_argument('j6',type=Path)
    p.add_argument('s1',type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.source_helpers.resolve()))
    w=types.ModuleType('shared_page_trial');w.sim=check
    sys.modules['shared_page_trial']=w
    f=a.offline_dir/'escape_lanes.py'
    sys.argv=[str(f),str(a.j6),str(a.s1),'--journal','--dense-head']
    runpy.run_path(str(f),run_name='__main__')
if __name__=='__main__':main()
