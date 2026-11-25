"""Offline SMK37 settings/panel migration. Operates only on inert 1 MiB files."""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import tempfile
import zlib
from dataclasses import dataclass

FLASH=1<<20
SEC=4096
PAGE=256
SET=(0x9D000,0x9E000)
PANEL=(0xA5000,0xA6000)
OWNER=0xAF000
OLD_TAG=b'SMK37DATAX0X'+bytes(4)
LOCS=SET+PANEL+(OWNER,)
OLD_MAGIC=0x30583058
J_MAGIC=b'DNJ1'
TEMP=512
ERASED=bytes([255])
PROFILES=('elite','pro')

def sha(b):return hashlib.sha256(b).hexdigest()
def crc(b):return zlib.crc32(b)
def erase(bs):return bs==ERASED*len(bs)
def sector(b,off):
    if len(b)!=FLASH:raise ValueError('expected complete 1 MiB flash image')
    return b[off:off+SEC]
def expect(p,msg):
    if not p:raise ValueError(msg)

@dataclass(frozen=True)
class Value:
    seq:int
    data:bytes

# Half-empty remains valid if other copy is good, as in the legacy reader.
def legacy(buf,kind,max_len):
    h=buf[:32]
    if erase(h):return None
    magic,typ,_slot,seq,n,dcrc,_r0,_r1,hcrc=struct.unpack('<IHH6I',h)
    expect(magic==OLD_MAGIC and typ==kind and crc(h[:28])==hcrc,'damaged legacy record header')
    expect(0<n<=max_len,'unknown legacy payload width')
    data=bytes(buf[PAGE:PAGE+n])
    expect(crc(data)==dcrc,'damaged legacy record body')
    return Value(seq,data)

def legacy_pair(buf,pair,kind,max_len):
    found=[(i,v) for i,a in enumerate(pair) if (v:=legacy(sector(buf,a),kind,max_len))]
    found.sort(key=lambda x:(x[1].seq,-x[0]),reverse=True)
    if len(found)==2 and found[0][1].seq==found[1][1].seq:
        # Runtime's A-on-tie rules for current; previous requires a strictly lower generation.
        expect(found[0][1].data==found[1][1].data,'ambiguous equal-generation legacy copies')
        found.pop()
    return ((found[0][0] if found else None), tuple(x[1] for x in found)+(None,)*(2-len(found)))

def legacy_snapshot(buf):
    expect(sector(buf,OWNER)[:16]==OLD_TAG,'unexpected legacy owner tag')
    idx,setting=legacy_pair(buf,SET,0,48)
    _,panel=legacy_pair(buf,PANEL,4,32)
    return idx,setting+panel

def encode(vals,seq,binding):
    expect(0<seq<=0xffffffff,'new journal sequence out of range')
    body=bytearray()
    for tag,v in enumerate(vals,1):
        d=v.data if v else b''
        n=v.seq if v else 0
        body.extend(struct.pack('<4I',tag,n,len(d),crc(d)))
        body.extend(d)
    expect(len(body)<=PAGE,'journal must use only one body page')
    expect(len(binding)==16,'invalid backup binding')
    h=struct.pack('<4sII16s',J_MAGIC,seq,len(body),binding)
    return h+struct.pack('<I',crc(h+body)),bytes(body)

def decode(buf):
    if len(buf)<2*PAGE or buf[:4]!=J_MAGIC:return None
    _m,seq,n,binding,dcrc=struct.unpack_from('<4sII16sI',buf)
    if not (seq and 64<=n<=PAGE):return None
    body=buf[PAGE:PAGE+n]
    if crc(buf[:28]+body)!=dcrc:return None
    vals=[]; off=0
    for tag in range(1,5):
        if off+16>n:return None
        t,oldseq,m,c=struct.unpack_from('<4I',body,off);off+=16
        if t!=tag or m>(48 if tag<=2 else 32) or off+m>n:return None
        d=bytes(body[off:off+m]);off+=m
        if crc(d)!=c or (not m and oldseq):return None
        vals.append(Value(oldseq,d) if m else None)
    return (seq,bytes(binding),tuple(vals)) if off==n else None

def reader(buf):
    """Candidate rolling-reset reader: regular journal wins; temp is durable migration fallback."""
    reg=[v for a in SET if (v:=decode(sector(buf,a)))]
    if reg:
        expect(len({v[1] for v in reg})==1,'conflicting journal identities')
        return max(reg,key=lambda r:r[0])[2]
    tmp=[v for a in SET if (v:=decode(sector(buf,a)[TEMP:]))]
    if tmp:
        expect(len({v[1] for v in tmp})==1,'conflicting temporary identities')
        return max(tmp,key=lambda r:r[0])[2]
    return legacy_snapshot(buf)[1]

def pack_backup(image,board):
    expect(board in PROFILES,'unknown board')
    legacy_snapshot(image)
    return dict(schema='dnbox-vm-migration-backup-v1',board=board,image_sha256=sha(image),
        sectors={hex(o):dict(sha256=sha(x),hex=x.hex()) for o in LOCS if (x:=sector(image,o))})
def backup_key(b):
    digest=bytes.fromhex(b['image_sha256'])
    expect(len(digest)==32,'bad source digest length')
    return digest[:16]

def verify_backup(b,board,source=None):
    backup_key(b)
    expect(b['schema']=='dnbox-vm-migration-backup-v1' and b['board']==board,'backup profile mismatch')
    expect(set(b['sectors'])=={hex(o) for o in LOCS},'missing/unexpected backup extent')
    out={}
    for o in LOCS:
        rec=b['sectors'][hex(o)];data=bytes.fromhex(rec['hex'])
        expect(len(data)==SEC and sha(data)==rec['sha256'],'damaged backup sector')
        if source is not None:expect(data==sector(source,o),'backup is not current device source')
        out[o]=data
    if source is not None:expect(sha(source)==b['image_sha256'],'entire source identity changed')
    return out

@dataclass(frozen=True)
class Op:
    name:str
    off:int
    data:bytes
    erase:bool=False

def write(buf,op,prefix=None):
    """NOR write to in-memory copy. prefix simulates byte-ordered torn operation."""
    n=len(op.data) if prefix is None else prefix
    expect(0<=n<=len(op.data),'bad cut')
    if op.erase:
        expect(op.off%SEC==0 and len(op.data)==SEC,'unaligned erase')
    else:
        expect(op.off//PAGE==(op.off+len(op.data)-1)//PAGE,'write crosses flash page')
        expect(all((a&b)==b for a,b in zip(buf[op.off:op.off+n],op.data[:n])),'programming a non-erased bit')
    buf[op.off:op.off+n]=op.data[:n]

def record_ops(off,vals,seq,binding,label):
    h,d=encode(vals,seq,binding)
    return (Op(label+' body',off+PAGE,d),Op(label+' commit',off,h))
def blank(off,label):return Op(label,off,ERASED*SEC,True)

def migrate(src,b,board):
    """Explicit planner. It never retires or writes any panel/sample/owner sector."""
    verify_backup(b,board,src)
    active,vals=legacy_snapshot(src)
    carrier=0 if active is None else active
    stage=SET[carrier]+TEMP
    expect(erase(src[stage:stage+2*PAGE]),'reserved legacy tail not erased; refusing')
    other=SET[1-carrier]
    key=backup_key(b)
    steps=[
      *record_ops(stage,vals,1,key,'temporary checkpoint'),
      blank(other,'first journal erase'),
      *record_ops(other,vals,1,key,'first journal'),
      blank(SET[carrier],'second journal erase'),
      *record_ops(SET[carrier],vals,2,key,'second journal'),
    ]
    result=bytearray(src)
    for op in steps:write(result,op)
    expect(all(decode(sector(result,a)) for a in SET) and reader(result)==vals,'journal readback failed')
    for off in range(0,FLASH,SEC):
        if off not in SET:expect(sector(result,off)==sector(src,off),'non-target sector changed')
    return bytes(result),steps

def resume(source,b,board):
    """Resume recognized migration interruption; other data/foreign writes fail closed."""
    saved=verify_backup(b,board)
    old=bytearray(source)
    for a in PANEL+(OWNER,):
        expect(sector(source,a)==saved[a],'untouched legacy sector changed')
    for a in SET:old[a:a+SEC]=saved[a]
    expect(sha(old)==b['image_sha256'],'non-journal source changed')
    final,steps=migrate(old,b,board)
    if final==source:return final,()
    a0=SET[0];a1=SET[1]+SEC
    observed=source[a0:a1]
    check=bytearray(old[a0:a1])
    idx=None
    for i,op in enumerate(steps):
        x=op.off-a0;y=x+len(op.data)
        if check[:x]==observed[:x] and check[y:]==observed[y:]:
            old_body=check[x:y];now=observed[x:y]
            # Maximum bit-prefix supported by the model: the present
            # prefix must equal newly programmed bytes, the suffix old bytes.
            min_prefix=max((j+1 for j,(a,c) in enumerate(zip(old_body,now)) if a!=c),default=0)
            max_prefix=next((j for j,(a,c) in enumerate(zip(op.data,now)) if a!=c),len(now))
            if min_prefix<=max_prefix:idx=i
        check[x:y]=op.data
    expect(idx is not None,'unknown interrupted journal state')
    result=bytearray(source)
    for op in steps[idx:]:write(result,op)
    expect(bytes(result)==final,'resume did not reach verified final image')
    return final,steps[idx:]

def may_retire(image,b,board):
    """Offline gate only. Does not write the reclaimed sectors or update firmware."""
    verify_backup(b,board)
    copies=[decode(sector(image,a)) for a in SET]
    return all(v and v[1]==backup_key(b) for v in copies) and copies[0][2]==copies[1][2] and abs(copies[0][0]-copies[1][0])==1

def update(image,kind,data):
    """Journal editor simulator. No write if checkpoint slots are not fully migrated."""
    expect(kind in ('settings','panel'),'unknown object')
    expect(0<len(data)<=(48 if kind=='settings' else 32),'invalid payload size')
    copies=[decode(sector(image,a)) for a in SET]
    expect(all(copies) and copies[0][1]==copies[1][1] and copies[0][0]!=copies[1][0],
           'incomplete/conflicting migration')
    newest=int(copies[1][0]>copies[0][0])
    generation,key,values=copies[newest]
    vals=list(values);k=0 if kind=='settings' else 2
    prior=vals[k]
    if prior and prior.data==data:return bytes(image),[]
    vals[k+1]=prior
    vals[k]=Value(prior.seq+1 if prior else 1,bytes(data))
    expect(vals[k].seq<=0xffffffff,'object sequence exhausted')
    dst=SET[1-newest]
    steps=(blank(dst,'update erase'),*record_ops(dst,vals,generation+1,key,'update'))
    out=bytearray(image)
    for op in steps:write(out,op)
    expect(reader(out)==tuple(vals),'edit readback failed')
    return bytes(out),steps

def rollback(src,b,board):
    """Restore legacy A/B and owner before old-app selection; no device connector."""
    sectors=verify_backup(b,board)
    key=backup_key(b)
    current=[v for a in SET if (v:=decode(sector(src,a)))]
    if not current:
        current=[v for a in SET if (v:=decode(sector(src,a)[TEMP:]))]
    expect(current and all(v[1]==key for v in current),'rollback backup lineage mismatch')
    out=bytearray(src)
    for a in LOCS:out[a:a+SEC]=sectors[a]
    restored=legacy_snapshot(out)[1]
    expect(reader(src)==restored,'new settings/panel edits need export before exact legacy rollback')
    return bytes(out)

def atomic_output(path,data):
    path=Path(path)
    expect(not path.exists(),'output already exists')
    with tempfile.NamedTemporaryFile(dir=path.parent,delete=False) as f:
        tmp=Path(f.name)
        try:
            f.write(data);f.flush();os.fsync(f.fileno())
            expect(tmp.read_bytes()==data,'file readback differs')
            # No overwrite even if a second process claimed the target meanwhile.
            os.link(tmp,path)
            fd=os.open(path.parent,os.O_RDONLY)
            try:os.fsync(fd)
            finally:os.close(fd)
        finally:
            tmp.unlink(missing_ok=True)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('verb',choices=('backup','migrate','resume','rollback'))
    p.add_argument('--board',choices=PROFILES,required=True)
    p.add_argument('image',type=Path)
    p.add_argument('output',type=Path)
    p.add_argument('--verified-backup',type=Path)
    a=p.parse_args()
    src=a.image.read_bytes()
    expect(len(src)==FLASH,'wrong flash size')
    if a.verb=='backup':
        atomic_output(a.output,(json.dumps(pack_backup(src,a.board),indent=2)+'\n').encode())
    else:
        expect(a.verified_backup is not None,'verified backup is mandatory')
        backup=json.loads(a.verified_backup.read_text())
        if a.verb=='migrate':out=migrate(src,backup,a.board)[0]
        elif a.verb=='resume':out=resume(src,backup,a.board)[0]
        else:out=rollback(src,backup,a.board)
        atomic_output(a.output,out)

if __name__=='__main__':main()
