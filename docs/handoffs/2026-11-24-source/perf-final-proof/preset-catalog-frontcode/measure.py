"""Read authorized corpora in memory; output only aggregate exact-encoding receipts."""
import ctypes as C
from pathlib import Path
import math,random,struct as S,subprocess,sys,tempfile,zlib
root=Path(sys.argv[1]).resolve()
loader=root/'perf-final-proof/sj6-combined-storage/measure-combined.py'
n={'__file__':str(loader)}
exec(compile(loader.read_text().split('def counters(subset):',1)[0],str(loader),'exec'),n)
bks=n['banks']
where=Path(__file__).resolve().parent
members=[r for b in bks for r in b[-1]]
print('authored bank/program fields S1/J6', 12,12,
      *[sum(len(b[-1]) for b in bks if b[0]==i) for i in (0,1)],
      n['sfields'],n['jfields'])
max_name=max(len(r[0]) for r in members)
print('actual max UTF-8 name bytes',max_name,
      'max expanded one J6 ordered record (one ID+one word per field)',max(5*len(r[2]) for b in bks if b[0]==1 for r in b[-1]))
print('complete 694+640 literal program UUID bytes',sum(len(r[1]) for r in members))
for e in (0,1):
    w=[b[-1] for b in bks if b[0]==e]
    for stripe in (8,16,32):
        mx=max(sum(2+len(name)+16+5*len(fields) for name,_,fields in rows[i:i+stripe]) for rows in w for i in range(0,len(rows),stripe))
        print("expanded cache e/S1=0 J6=1, restart, max exact id+word/name/UUID bytes",e,stripe,mx)
print('report arithmetic, supplied benchmark only: 61,052 tuned S1 + 79,195 externally reported J6 words =',61052+79195)
print('bank directory includes 88 B fixed header + pinned corpus IDs, per-bank 12 B fixed fields, exact title and authored J6 bank UUID; 16-bit per-block offsets')
print('stored UUID variant charges 16 bytes per program in addition; CRC lives in the 88 B header')
assert max_name<255
def pack(rows,shift):
    offsets=bytearray(); code=bytearray(); old=bytearray(); previous=b''
    for i,(name,_,_) in enumerate(rows):
        if i%(1<<shift)==0:
            offsets+=S.pack('<H',len(code));previous=b''
        p=0
        while p<min(len(previous),len(name)) and previous[p]==name[p]:p+=1
        s=len(name)-p
        old+=bytes([p,s])+name[p:]
        code.append((min(p,15)<<4)|min(s,15))
        if p>=15:code.append(p)
        if s>=15:code.append(s)
        code+=name[p:]
        previous=name
    return bytes(offsets),bytes(code),len(old)
with tempfile.TemporaryDirectory(dir='/tmp') as t:
    lib=Path(t)/'reader.so'
    subprocess.run(['cc','-O2','-fPIC','-shared',str(where/'catalog-reader.c'),'-o',str(lib)],check=True)
    fn=C.CDLL(str(lib)).cat_name
    fn.argtypes=[C.c_char_p,C.c_uint,C.c_char_p,C.c_uint,C.c_uint,C.c_uint,C.c_uint,C.c_char_p,C.c_uint]
    rng=random.Random(2427)
    for shift in (2,3,4,5):
        groups=[]; labels=older=offset_bytes=0
        # 24-byte format header (version/counts/section boundaries/CRC) and 64-byte corpus digest.
        reg=88+sum(12+len(b[1])+len(b[2]) for b in bks)
        for e,title,bank_id,first,rows in bks:
            ofs,bb,old=pack(rows,shift)
            labels+=len(bb); older+=old; offset_bytes+=len(ofs)
            groups.append((ofs,bb,rows))
        tests=[(g,j) for g,b in enumerate(groups) for j in range(len(b[2]))]
        rng.shuffle(tests)
        hits=0
        for g,j in tests:
            ofs,bb,rows=groups[g]
            args=(ofs,len(ofs)//2,bb,len(bb),shift,len(rows),j)
            out=C.create_string_buffer(max_name+1)
            assert fn(*args,out,len(out)) and out.value==rows[j][0]
            assert not fn(*args,out,0)
            # The last requested name's block must not accept a bad block start.
            i=j>>shift; bad=ofs[:i*2]+b'\xff\xff'+ofs[i*2+2:]
            assert not fn(bad,len(ofs)//2,bb,len(bb),shift,len(rows),j,out,len(out))
            hits+=1
        # Old and new formats both charge an equally-sized per-block directory.
        total=labels+offset_bytes+reg
        print('seek',1<<shift,'tested rows',hits,'name bytes compact/2-byte',labels,older,
              'offsets',offset_bytes,'registry/header',reg,'catalog implicit',total,
              'catalog UUIDs',total+16*len(members),'saved',older-labels)
    # Measure actual standalone target name decoder; does not claim boot/parser XIP.
    tc=root/'prereqs/toolchain'; obj=Path(t)/'reader.o'
    subprocess.run([str(tc/'pi32v2/bin/clang'),'-target','pi32v2','-Oz','-ffreestanding',
                    '-fno-builtin','-c',str(where/'catalog-reader.c'),'-o',str(obj)],check=True)
    out=subprocess.check_output(['size',str(obj)],text=True)
    print('standalone native name reader object text/data/bss',*out.splitlines()[-1].split()[:3])
    # Verified numerical charge only: new parser/container natives not constructed here.
    print('foreground scratch',max_name+1,'caller-owned output bytes; no history/dictionary/heap')
