#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Independent host NOR model: installation, deletion and interrupted updates."""
import ctypes as C
import os
from pathlib import Path
import subprocess
import tempfile
import zlib
from encode import encode

SECTOR = 4096
SIZE = SECTOR * 30
P, I, U = C.c_void_p, C.c_uint32, C.c_uint
E = C.CFUNCTYPE(C.c_int, P, I)
W = C.CFUNCTYPE(C.c_int, P, I, P, U)
H = C.CFUNCTYPE(C.c_int, P)
A = C.CFUNCTYPE(C.c_int, P, P, P)
X = C.CFUNCTYPE(I, P)


class IO(C.Structure):
    _fields_ = [('flash', P), ('ctx', P), ('erase', E), ('write', W), ('held', H), ('auth', A), ('extent', X)]


class Info(C.Structure):
    _fields_ = [('generation', I), ('reserved', C.c_uint8), ('banks', C.c_uint8), ('bank_sectors', C.c_uint8), ('free_sectors', C.c_uint8), ('contiguous', C.c_uint8)]


def run(lib):
    for name, args in {'init':[P,P], 'format':[P], 'adopt_sample':[P], 'get_info':[P,P], 'verify':[P], 'begin':[P,P,I,I,I], 'write':[P,I,P,U], 'end':[P], 'remove':[P,P,I], 'reserve_sample':[P,U,I], 'release_sample':[P,I], 'bank_at':[P,U,P,P,P], 'bank_read':[P,P,I,I,P,U]}.items():
        getattr(lib, 'dnp_'+name).argtypes = args
    lib.dnp_init.restype = None

    class Model:
        def __init__(self, image=None):
            self.mem = (C.c_uint8*SIZE).from_buffer_copy(image if image is not None else b'\xff'*SIZE)
            self.pool = C.create_string_buffer(256)
            self.ops = []
            self.cut, self.partial = 0, 0
            self.extent = 0
            self.allow = True
            self.io = IO(C.addressof(self.mem), None, E(lambda _, o: self.op(o, b'\xff'*SECTOR, True)), W(lambda _, o, p, n: self.op(o, C.string_at(p,n), False)), H(lambda _: 1), A(lambda *_: self.allow), X(lambda _: self.extent))
            lib.dnp_init(self.pool, C.byref(self.io))
            if image is None:
                assert lib.dnp_format(self.pool) == 0
            self.ops.clear()

        def op(self, off, data, erase):
            n = len(data)
            assert 0 <= off <= SIZE-n and (off % SECTOR == 0 and n == SECTOR if erase else 0 < n <= 256)
            if not erase:
                assert all(a & b == b for a,b in zip(self.mem[off:off+n],data))
            self.ops.append(n)
            failed = self.cut and len(self.ops) >= self.cut
            k = min(n,self.partial) if failed and len(self.ops)==self.cut else 0 if failed else n
            self.mem[off:off+k] = data[:k]
            return -1 if failed else 0

        def info(self):
            out = Info()
            assert lib.dnp_get_info(self.pool,C.byref(out)) == 0
            return out

        def install(self, blob):
            rc = lib.dnp_begin(self.pool,blob[16:24],len(blob),zlib.crc32(blob),self.info().generation)
            if rc:
                return rc
            for o in range(0,len(blob),256):
                data = blob[o:o+256]
                rc = lib.dnp_write(self.pool,o,data,len(data))
                if rc:
                    return rc
            return lib.dnp_end(self.pool)

        def remove(self,key):
            return lib.dnp_remove(self.pool,key,self.info().generation)

        def reserve(self,n):
            return lib.dnp_reserve_sample(self.pool,n,self.info().generation)

        def packs(self):
            out = {}
            info = self.info()
            assert lib.dnp_verify(self.pool)==0
            for i in range(info.banks):
                key, size, crc = C.create_string_buffer(8), I(), I()
                assert lib.dnp_bank_at(self.pool,i,key,C.byref(size),C.byref(crc))==0
                blob = bytearray()
                for off in range(0,size.value,256):
                    n = min(256,size.value-off)
                    buf = C.create_string_buffer(n)
                    assert lib.dnp_bank_read(self.pool,key,info.generation,off,buf,n)==0
                    blob += buf.raw
                assert zlib.crc32(blob)==crc.value
                out[key.raw] = bytes(blob)
            return out

    def faults(image, action, before, after):
        baseline = Model(image)
        assert action(baseline)==0
        count = 0
        for i,n in enumerate(baseline.ops,1):
            for k in (0, n//2, n-1):
                m = Model(image)
                m.cut, m.partial = i, k
                assert action(m)!=0
                assert Model(bytes(m.mem)).packs() in (before,after)
                count += 1
        return count

    key = b'bank0001'
    rows = [(i.to_bytes(8,'little'), f'Preset {i}', [(j, 0x3f000000 + i*128+j) for j in range(40)]) for i in range(100)]
    full = encode(2,key,'Test bank',rows)
    small = encode(2,key,'Test bank',rows[:5])
    other = encode(3,b'bank0002','Second bank',rows)
    m = Model()
    assert m.install(full)==0 and m.install(other)==0
    before = m.packs()
    m.allow = False
    assert m.install(small)==-6 and m.packs()==before
    assert m.remove(key)==-6 and m.packs()==before
    m.allow = True
    after = dict(before); after[key] = small
    cuts = faults(bytes(m.mem),lambda x:x.install(small),before,after)
    assert m.install(small)==0 and m.packs()==after
    before = m.packs(); after = {key:small}
    cuts += faults(bytes(m.mem),lambda x:x.remove(b'bank0002'),before,after)
    assert m.remove(b'bank0002')==0 and m.packs()==after
    cap = 28-m.info().bank_sectors
    cuts += faults(bytes(m.mem),lambda x:x.reserve(cap),after,after)
    assert m.reserve(cap)==0 and m.info().contiguous==cap and m.packs()==after
    assert m.remove(key)==0
    assert lib.dnp_release_sample(m.pool,m.info().generation)==0 and m.info().contiguous==28
    # Migration of an existing sample: the extent callback is the trusted
    # FSMP/CRC/shadow authority; no sample bytes may be changed by adoption.
    extent = 2*SECTOR-37
    sample = bytes((i*31+17)&255 for i in range(extent))
    legacy = sample + b'\xff'*(SIZE-extent)
    def adopt(image=legacy, end=extent):
        m = Model(image);m.extent=end;return m
    a = adopt();assert lib.dnp_adopt_sample(a.pool)==0
    assert a.info().reserved==2 and a.info().banks==0
    assert bytes(a.mem[:extent])==sample
    assert a.packs()=={}
    nops=list(a.ops); a.ops.clear();assert lib.dnp_adopt_sample(a.pool)==0 and not a.ops
    migration_cuts=0
    for i,n in enumerate(nops,1):
        for k in sorted({0,1,n//2,n-1,n}):
            a=adopt();a.cut,a.partial=i,k;assert lib.dnp_adopt_sample(a.pool)!=0
            r=adopt(bytes(a.mem));assert lib.dnp_adopt_sample(r.pool)==0
            assert bytes(r.mem[:extent])==sample and r.info().reserved==2 and r.packs()=={}
            migration_cuts+=1
    for offset in (2*SECTOR,15*SECTOR,28*SECTOR+50,29*SECTOR):
        bad=bytearray(legacy);bad[offset:offset+4]=b'S1PK';a=adopt(bad)
        assert lib.dnp_adopt_sample(a.pool)==-6 and bytes(a.mem)==bad and not a.ops
    for invalid_extent in (0,28*SECTOR+1,0xffffffff):
        a=adopt(end=invalid_extent);assert lib.dnp_adopt_sample(a.pool)==-6 and bytes(a.mem)==legacy
    a=adopt();assert lib.dnp_adopt_sample(a.pool)==0 and a.install(full)==0
    before=bytes(a.mem);assert lib.dnp_adopt_sample(a.pool)==-6 and bytes(a.mem)==before
    print('PASS:',migration_cuts,'legacy-sample adoption interruption cases; S1PK, uncertain shadow, oversized sample and nonempty directory refused')
    print('PASS: exact installed bytes, protected deletion, subset replacement, relocation, full capacity,',cuts,'interrupted writes')


if __name__ == '__main__':
    here = Path(__file__).resolve().parent
    with tempfile.TemporaryDirectory() as temp:
        so = Path(temp)/'pool.so'
        subprocess.run([os.environ.get('CC','cc'),'-std=c11','-Os','-shared','-fPIC','-fsanitize=undefined','-fno-sanitize-recover=all',*[str(here/f) for f in ('dnbank.c','dnpool.c','dnpwire.c')],'-o',str(so)],check=True)
        run(C.CDLL(str(so)))
