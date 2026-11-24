"""Compile and verify the best DNP predictor's separate COPY stage on real rows."""
import ctypes as C, hashlib, struct, subprocess, sys,tempfile
from pathlib import Path
R=Path(sys.argv[1]).resolve(); code=Path(sys.argv[2]).resolve(); sys.path.insert(0,str(code))
import predictor_study as p
import benchmark as b
bank=R/'sj6-private-final/J6presets(2).zip'
assert hashlib.sha256(bank.read_bytes()).hexdigest()=='ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8'
rows,meta=p.load_j6(bank);assert sorted(meta['member_sha256'])==sorted(p.J6_HASHES)
l=[]
def capture(r):
    pack,man,rep=b.run(r);l.extend([pack,man]);return pack,man,rep
p.run=capture
rep,refs=p.predict(rows,False);blob,man=l
fields={tuple(k):i for i,k in enumerate(man['field_ids'])}
rf=sorted((fields[e,n],fields[e,ref]) for e,n,ref,_,_ in refs)
dsts=dict(rf)
assert len(fields)==174 and len(rf)==52
with tempfile.TemporaryDirectory(dir='/tmp') as t:
    t=Path(t);c=Path(__file__).with_name('restore-copy.c')
    so=t/'copy.so'
    subprocess.run(['cc','-O2','-fPIC','-shared',str(c),'-o',str(so)],check=True)
    fn=C.CDLL(str(so)).restore_copy
    u16=C.c_uint16;u32=C.c_uint32
    fn.argtypes=[C.POINTER(u16),C.POINTER(u32),C.c_uint,C.POINTER(u16),C.c_uint]
    ref=(u16*(2*len(rf)))(*(x for pair in rf for x in pair))
    copied=steps=worst=0
    for prog,dec in zip(rows,b.unpack(blob),strict=True):
        entries=dec[-1]; layout=(u16*len(entries))(*(f for f,_ in entries)); staged=(u32*len(entries))(*(w for _,w in entries))
        assert len(entries)==len(prog.fields)
        scan=0
        for i,(f,w) in enumerate(entries):
            if w!=p.COPY: continue
            k=i-1
            while k>=0 and entries[k][0]!=dsts[f]:k-=1
            assert k>=0
            scan+=i-k
            copied+=1
        steps+=scan;worst=max(worst,scan)
        assert fn(layout,staged,len(entries),ref,len(rf))
        assert list(staged)==[w for _,w in prog.fields]
    assert not fn((u16*1)(rf[0][0]),(u32*1)(p.COPY),1,ref,len(rf))
    assert not fn((u16*1)(0),(u32*1)(p.PREV),1,ref,len(rf))
    assert not fn((u16*1)(174),(u32*1)(0),1,ref,len(rf))
    print('exact COPY token restoration C, rows, tokens, backward field probes total/worst',len(rows),copied,steps,worst)
    tc=R/'prereqs/toolchain/pi32v2/bin';obj=t/'copy.o'
    subprocess.run([str(tc/'clang'),'-target','pi32v2','-Oz','-ffreestanding','-fno-builtin','-c',str(c),'-o',str(obj)],check=True)
    print('standalone COPY-only PI32 object text/data/BSS',*subprocess.check_output(['size',str(obj)],text=True).splitlines()[-1].split()[:3])
    print('native stack opcodes',*[x.split('\t')[-1] for x in subprocess.check_output([str(tc/'objdump'),'-d',str(obj)],text=True).splitlines() if '[--sp]' in x or '[sp++]' in x])
    peak=max(len(r.fields) for r in rows)
    print('no dictionary/heap; shared caller staged words',peak*4,'layout words',peak*2,'sparse refs',len(rf)*4)
