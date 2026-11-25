"""Synthetic emulator-only stream. Exact project wire is checked before writes."""
from pathlib import Path
import argparse,json,re,struct,subprocess,zlib
import guarded_replay as g
def native_project(elf,ram,size):
    text=subprocess.check_output(['readelf','-sW',elf],text=True)
    m=re.search(r'^\s*\d+:\s+([a-fA-F0-9]+)\s+(\d+) OBJECT .*\bproj$',text,re.M)
    assert m and int(m[2])==size
    data=g.regular(ram); at=int(m[1],16)-g.ELF_RAM
    assert 0<=at<=len(data)-size
    return data[at:at+size]
def build(q):
    cmd=[str(q.fixture)]
    sz,b,sn,rawat,cell,arp,tag,extra=map(int,subprocess.check_output(cmd+['layout'],text=True).split())
    assert (sz,b,sn,rawat,cell,arp,tag,extra)==(36420,40,2712,2752,768,832,2688,960)
    p=bytearray(native_project(q.elf,q.ram,sz))
    assert p[:4]==b'0X0P' and g.u32(p,4)==1 and p[19]==13
    assert g.sha(g.regular(q.owner_elf))=='3e78835b5c9928c68b4da092eb8c7677d2e37f07b47edc27c22ad795ae9d7c84'
    owner=native_project(q.owner_elf,q.owner_ram,sz)
    assert p[b+cell+32:b+cell+64]==owner[b+cell+32:b+cell+64]
    assert owner[b+cell+29:b+cell+31]==b'\x02\x00'
    assert owner[b+arp:b+arp+3]==b'A\x01\x09'
    assert owner[b+tag:b+tag+4]==b'11SO'
    for off,n in ((cell,32),(arp,32),(tag,12)):
        at=b+off;p[at:at+n]=owner[at:at+n]
    if q.active:
        a=subprocess.check_output(cmd+['room'],input=p)
        assert len(a)==sz and a[b+tag+10]==1
        assert all(x==y or any(b+off<=i<b+off+n for off,n in ((cell,32),(extra,32),(tag,12)))
                   for i,(x,y) in enumerate(zip(p,a)))
        p=a
    raw=bytes(p[rawat:]); packed=subprocess.check_output(cmd+['encode'],input=raw)
    prev=g.regular(q.nor);nor=bytearray(prev)
    assert len(nor)==g.FLASH_SIZE and len(packed)<4*3836-12
    chunks=[struct.pack('<4I',13,len(raw),zlib.crc32(raw),len(packed))+packed[:3824]]; packed=packed[3824:]
    for _ in range(3):
        chunks.append(struct.pack('<I',13)+packed[:3836]);packed=packed[3836:]
    assert not packed
    def put(obj,data):
        off=0x9d000+obj*8192
        assert nor[off:off+4096]==b'\xff'*4096 and 4<=len(data)<=3840
        head=struct.pack('<IHH5I',0x30583058,obj,0,1,len(data),zlib.crc32(data),0xffffffff,0xffffffff)
        nor[off:off+32]=head+struct.pack('<I',zlib.crc32(head))
        nor[off+256:off+256+len(data)]=data
    put(0,struct.pack('<I',1)+p[:b]);put(1,struct.pack('<I',1)+p[b:rawat])
    for k,d in zip((2,3,5,6),chunks):put(k,d)
    assert nor[0xaf000:0xaf010]==b'\xff'*16
    nor[0xaf000:0xaf010]=b'SMK37DATAX0X\0\0\0\0'
    assert all(x==y or (0x9d000<=i<0xa5000 or 0xa7000<=i<0xab000 or 0xaf000<=i<0xaf010)
               for i,(x,y) in enumerate(zip(nor,prev)))
    return bytes(nor)
if __name__=='__main__':
    a=argparse.ArgumentParser()
    for n in ('nor','elf','ram','owner_elf','owner_ram','fixture','output'):a.add_argument(n,type=Path)
    a.add_argument('--active',action='store_true');q=a.parse_args()
    assert q.output.resolve().is_relative_to('/tmp') and q.output.suffix=='.bin'
    out=build(q)
    with q.output.open('xb') as f:f.write(out)
    print(json.dumps({'seed_sha256':g.sha(out),'cold_native':g.sha(g.regular(q.elf)),'owner_native':g.sha(g.regular(q.owner_elf))}))
