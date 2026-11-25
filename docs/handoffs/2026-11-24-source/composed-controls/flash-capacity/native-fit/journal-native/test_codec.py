"""Real Python wire bytes compared against host-compiled native specimen source."""
import ctypes as C
import os,subprocess,sys,tempfile
from pathlib import Path
W=Path(__file__).resolve().parent
R=W.parents[2]
sys.path.insert(0,str(W.parent))
import migration as py
from test_migration import fixture,data
class Snapshot(C.Structure):
    _fields_=[('binding',C.c_ubyte*16),('lens',C.c_ubyte*4),('data',C.c_ubyte*160),
              ('generation',C.c_uint32),('seq',C.c_uint32*4)]
READ=C.CFUNCTYPE(C.c_int,C.c_void_p,C.c_uint32,C.c_void_p,C.c_uint)
with tempfile.TemporaryDirectory() as d:
    obj=Path(d)/'journal.so'
    subprocess.run(['cc','-O2','-Wall','-Werror','-shared','-fPIC','-I',str(R/'fw/firmware/src'),str(W/'journal.c'),'-o',str(obj)],check=True)
    c=C.CDLL(str(obj));c.vmj_select.argtypes=(READ,C.c_void_p,C.POINTER(Snapshot))
    c.vmj_encode.argtypes=(C.POINTER(Snapshot),C.c_void_p,C.c_void_p)
    def read(image,fault=None):
        captures=[]
        @READ
        def f(cx,off,dst,n):
            assert off+n<=len(image)
            captures.append((off,n))
            if fault==off:return -1
            C.memmove(dst,image[off:off+n],n);return 0
        s=Snapshot();r=c.vmj_select(f,None,C.byref(s))
        return r,s,captures
    def convert(s):
        starts=[0,48,96,128]
        return tuple(py.Value(s.seq[i],bytes(s.data[starts[i]:starts[i]+s.lens[i]])) if s.lens[i] else None for i in range(4))
    compared=0
    for board in py.PROFILES:
        for active in (0,1):
            for missing in (None,0,4):
                orig=fixture(active,missing);bak=py.pack_backup(orig,board)
                modified,ops=py.migrate(orig,bak,board)
                im=bytearray(orig)
                for op in ops:
                    for cut in sorted({0,1,len(op.data)//2,len(op.data)-1,len(op.data)}):
                        sample=bytearray(im);py.write(sample,op,cut)
                        r,s,rr=read(bytes(sample))
                        assert r in (0,1) and convert(s)==py.reader(sample),(r,op,cut)
                        compared+=1
                    py.write(im,op)
                updated,_=py.update(modified,'settings',data(48,242))
                updated,_=py.update(updated,'panel',data(32,244))
                for pair in [orig,modified,updated]:
                    r,s,rr=read(pair)
                    assert r in (0,1) and convert(s)==py.reader(pair)
                    if s.generation:
                        h=C.create_string_buffer(32);body=C.create_string_buffer(224)
                        n=c.vmj_encode(C.byref(s),h,body)
                        eh,eb=py.encode(py.reader(pair),s.generation,bytes(s.binding))
                        assert h.raw==eh and body.raw[:n]==eb
                        if pair==updated and missing is None and active==1:
                            assert n==224
                    for off,_ in set(rr):
                        er,_,_=read(pair,off)
                        assert er==-2
    print('PASS: Python/C exact histories, payload CRC, max 224, callback I/O failures;',compared,'stage-cut fixture comparisons; sizeof snapshot =',C.sizeof(Snapshot))
