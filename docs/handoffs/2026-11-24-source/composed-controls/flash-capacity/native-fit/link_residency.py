"""Use the real JieLi PI32v2 linker to test exact asset occupancy.

This builds a data-only lab ELF from source bytes and emits only a receipt.
It does not compile the firmware, change the device or generate flash images.
"""
import argparse
import importlib.util
import json
import lzma
from pathlib import Path
import re
import runpy
import struct
import subprocess
import sys
import tempfile
import types
import zlib

SECTOR=4096

def pad(b,n=SECTOR,fill=b'\xff'):
    return b+fill*((-len(b))%n)

def model_and_banks(a):
    captured={}
    def ordered(model,replaced,banks,target,old,new,decode,fresh):
        original=dict(replaced)
        original[target]=old
        fmt=struct.pack('<4sIII',b'DML1',len(model),
            len(lzma.decompress(model,format=lzma.FORMAT_RAW,
                filters=[dict(id=lzma.FILTER_LZMA2,dict_size=65536)])),
            zlib.crc32(model))
        storage=bytearray(fmt+model)
        ext=[]
        for k,(bid,rows) in banks.items():
            b=original[k]
            assert decode(k[0],bid,b)==rows
            storage.extend(b'\xff'*((-len(storage))%4))
            ext.append((bid,len(storage),len(b),zlib.crc32(b)))
            storage.extend(b)
        assert len(ext)==25 and sum(len(v[1])for v in banks.values())==1546
        body=b''.join(struct.pack('<8sIII',*t)for t in ext)
        directory=lambda sequence:struct.pack('<4sIII',b'DRLE',sequence,len(body),zlib.crc32(body))+body
        captured.update(bank=bytes(storage),dirs=pad(directory(1))+pad(directory(0)))
    fake=types.ModuleType('shared_page_trial');fake.sim=ordered
    sys.modules['shared_page_trial']=fake
    sys.path.insert(0,str(a.helpers))
    f=a.offline/'escape_lanes.py'
    sys.argv=[str(f),str(a.superj6),str(a.synthone),'--journal','--dense-head']
    runpy.run_path(str(f),run_name='__main__')
    assert len(captured['bank'])==178938 and len(captured['dirs'])==8192
    return captured

def codec(path):
    m=types.ModuleType('audio_codec')
    s=path.read_text()
    before='    return len(payload)'
    assert s.count(before)==1
    exec(compile(s.replace(before,'    return payload'),str(path),'exec'),m.__dict__)
    return m

def factory_data(a,c):
    indexed={1:[],2:[]}
    samples=a.firmware/'assets/default-breaks'
    import hashlib
    for j in json.loads((samples/'default-breaks.json').read_text())['assets']:
        b=(samples/(j['id']+'.adpcm')).read_bytes()
        assert hashlib.sha256(b).hexdigest()==j['adpcm_sha256']
        indexed[j['slot']].append(b)
    br={}
    for k,clips in indexed.items():
        payload=[min((c.huff_asset(b,256,i,False)for i in (16,256)),key=len)for b in clips]
        header=struct.pack('<4sB',b'DBWH',len(clips))+b''.join(struct.pack('<I',len(p))for p in payload)
        br[k]=header.ljust(512,b'\x00')+b''.join(payload)
    assert [len(br[i])for i in (1,2)]==[55239,46037]
    # Use each board's produced stock sample header as a separate exact witness.
    d9={}
    head=lambda board:a.firmware/'build'/board/'gen/x0x_drum_samples.h'
    for board in ('smk37-elite','smk37'):
        rows=re.findall(r'uint8_t (x0x_smp_(?:crash|ride|hh))\[(\d+)\] = \{([^}]+)\}',head(board).read_text())
        pack=[]
        source=[]
        for n,k,v in rows:
            b=bytes(int(i.strip(),0)for i in v.split(',')if i.strip())
            assert len(b)==int(k)
            pack.append(c.huff_asset(b,256,256,False))
            source.append(b)
        assert len(pack)==3 and sum(map(len,source))==55275 and sum(map(len,pack))==50961
        d9[board]=b''.join(pack)
    assert d9['smk37-elite']==d9['smk37']
    return br,d9['smk37']

def addresses(*ranges):
    return [start for lo,hi in ranges for start in range(lo,hi,SECTOR)]

def linker(trial,data,pool,tc,d,expected):
    assert len(set(pool))==len(pool)
    data=pad(data)
    assert d.is_dir() and len(data)%SECTOR==0
    pages=len(data)//SECTOR
    excluded=pages-min(len(pool),pages)
    assert excluded*SECTOR==expected
    (d/'bytes.bin').write_bytes(data)
    asm=[f'.section .f{i:03d},"a"\n' + ('.global _page_000\n_page_000:\n' if not i else '')+
        f'.incbin "bytes.bin",{i*SECTOR},{SECTOR}\n' for i in range(pages)]
    (d/'payload.S').write_text(''.join(asm))
    clang=tc/'common/bin/clang'
    ld=tc/'pi32v2/bin/ld'
    subprocess.run([str(clang),'-target','pi32v2','-mcpu=r3','-c','payload.S','-o','payload.o'],cwd=d,check=True)
    mem=[f' R{i} (r) : ORIGIN = {addr:#x}, LENGTH = {SECTOR:#x}' for i,addr in enumerate(pool[:pages])]
    mem.append(' DENY (r) : ORIGIN = 0x03000000, LENGTH = 0')
    sec=[f' .f{i:03d} : {{ *(.f{i:03d}) }} > R{i}' for i in range(min(len(pool),pages))]
    overflow=','.join(f'.f{i:03d}' for i in range(len(pool),pages)).replace(',',' ')
    if overflow:
        sec.append(' .overflow : { *('+overflow+') } > DENY')
    (d/'fit.ld').write_text('MEMORY {\n'+'\n'.join(mem)+'\n}\nSECTIONS {\n'+'\n'.join(sec)+'\n}\n')
    r=subprocess.run([str(ld),'-T','fit.ld','-e','_page_000','payload.o','-o','never-install.elf'],
                       cwd=d,capture_output=True,text=True)
    if expected:
        assert r.returncode!=0 and re.search(r"region .DENY. overflowed by %d bytes"%expected,r.stderr),r.stderr
    else:
        assert r.returncode==0,r.stderr
        elf=(d/'never-install.elf').read_bytes()
        w=subprocess.check_output(['readelf','-SW','never-install.elf'],cwd=d,text=True)
        sec=re.findall(r'\.f\d{3}\s+PROGBITS\s+(\w+)\s+(\w+)\s+(\w+)',w)
        assert len(sec)==pages
        for i,(va,off,n) in enumerate(sec):
            v,o,k=map(lambda a:int(a,16),(va,off,n))
            assert v==pool[i] and k==SECTOR and elf[o:o+k]==data[i*k:i*k+k]
    return dict(trial=trial,link_exit_code=r.returncode,physical_pool_KiB=len(pool)*4,
                demand_KiB=pages*4,exact_native_overflow_B=expected,diagnostic=r.stderr.strip())

def main():
    p=argparse.ArgumentParser()
    for name in ('offline','helpers','superj6','synthone','firmware','codec','toolchain'):
        p.add_argument('--'+name,required=True,type=Path)
    x=p.parse_args()
    for k,v in vars(x).items():setattr(x,k,v.resolve())
    data=model_and_banks(x)
    br,d9=factory_data(x,codec(x.codec))
    # A/B have real sector-cost placeholders and source-bank identities.
    # This broad case deliberately grants the unproven full VM release and
    # all three sample slots, with the fixed 909/Chicago elsewhere.
    generous=addresses((0x9d000,0x9f000),(0xa5000,0xa7000),(0xb3000,0xbb000),(0xc2000,0xf4000))
    # Conditional conservative map already charges BREAK, FDATA and dirs.
    lower=addresses((0xa5000,0xa7000),(0xe2000,0xe4000))
    bank=pad(data['bank'])
    breaks=pad(br[1]+br[2])
    trialset=[
        ('control: exact assets in an oversized private address window',bank+breaks+data['dirs'],
         addresses((0x02000000,0x02047000)),0),
        ('generous: all three slots plus 16KiB VM, fixed assets outside',
         bank+breaks+data['dirs'],generous,36*1024),
        ('same generous layout + tested full-bank update reserve',
         bank+breaks+data['dirs']+bytes(20*1024),generous,56*1024),
        ('conservative vacant preset sectors after preserving existing BREAK/909',
         bank,lower,160*1024),
    ]
    with tempfile.TemporaryDirectory(prefix='dnbox-data-only-link-') as tmp:
        rows=[]
        for i,(trial,b,w,err) in enumerate(trialset):
            dest=Path(tmp)/str(i);dest.mkdir()
            rows.append(linker(trial,b,w,x.toolchain,dest,err))
        print(json.dumps(dict(data_only=True,preset_raw=len(data['bank']),break_raw=sum(map(len,br.values())),
            exact_repacked_909_B=len(d9),trial_rows=rows),indent=2))

if __name__=='__main__':main()
