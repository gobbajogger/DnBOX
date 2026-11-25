"""Python unittest test of the exported offline migration state machine."""
import hashlib, json, subprocess, struct, sys, tempfile, unittest
from copy import deepcopy
from pathlib import Path

import migration as m

def data(n,seed):return bytes((i*49+19*seed)&255 for i in range(n))
def legacy_sector(kind,slot,seq,payload):
    prefix=struct.pack('<IHH5I',m.OLD_MAGIC,kind,slot,seq,len(payload),m.crc(payload),0xffffffff,0xffffffff)
    header=prefix+struct.pack('<I',m.crc(prefix))
    return header+m.ERASED*(m.PAGE-len(header))+payload+m.ERASED*(m.SEC-m.PAGE-len(payload))
def fixture(active=0,missing=None):
    src=bytearray(b'\x6d'*m.FLASH)
    for pair,tag,lens in ((m.SET,0,(44,48)),(m.PANEL,4,(32,32))):
        for i,a in enumerate(pair):
            src[a:a+m.SEC]=(m.ERASED*m.SEC if missing==tag else
                legacy_sector(tag,i,9 if i==active else 8,data(lens[i],i+tag)))
    src[m.OWNER:m.OWNER+m.SEC]=m.OLD_TAG+m.ERASED*(m.SEC-len(m.OLD_TAG))
    return bytes(src)

def cuts(src,ops,values):
    img=bytearray(src); count=0
    for op in ops:
        start=op.off; end=start+len(op.data)
        before=bytes(img[start:end])
        for n in range(len(op.data)+1):
            m.write(img,op,n)
            assert m.reader(img) in values,(op.name,n)
            count+=1
            img[start:end]=before
        m.write(img,op)
    return count

class Tests(unittest.TestCase):
    def test_roundtrip_all_physical_pairs(self):
        n=0
        for board in m.PROFILES:
            for active in (0,1):
                src=fixture(active)
                _i,old=m.legacy_snapshot(src); b=m.pack_backup(src,board)
                img,ops=m.migrate(src,b,board)
                self.assertEqual(m.reader(img),old)
                self.assertEqual(m.rollback(img,b,board),src)
                self.assertTrue(m.may_retire(img,b,board))
                self.assertFalse(m.may_retire(src,b,board))
                self.assertEqual({a for a in range(0,m.FLASH,m.SEC) if m.sector(img,a)!=m.sector(src,a)},set(m.SET))
                n+=cuts(src,ops,{old})
        print('migration reset points:',n)

    def test_both_key_histories_preserved(self):
        n=0
        for which in ('settings','panel'):
            src=fixture(); b=m.pack_backup(src,'pro'); oldimg,_=m.migrate(src,b,'pro')
            old=m.reader(oldimg)
            newimg,operations=m.update(oldimg,which,data(48 if which=='settings' else 32,9))
            new=m.reader(newimg);i=0 if which=='settings' else 2
            self.assertEqual(new[i+1],old[i])
            self.assertEqual(new[2-i:4-i],old[2-i:4-i])
            n+=cuts(oldimg,operations,{old,new})
            with self.assertRaisesRegex(ValueError,'edits need export'):
                m.rollback(newimg,b,'pro')
        print('edit reset points:',n)

    def test_refusals_are_nonmutating(self):
        src=fixture(); b=m.pack_backup(src,'elite')
        b2=deepcopy(b);b2['sectors'][hex(m.PANEL[0])]['hex']='12'*m.SEC
        for bs,board in ((b2,'elite'),(b,'pro')):
            with self.assertRaises(ValueError):m.migrate(src,bs,board)
        altered=bytearray(src);altered[m.SET[0]+m.TEMP]=0
        with self.assertRaises(ValueError):m.migrate(altered,b,'elite')
        own=m.pack_backup(bytes(altered),'elite')
        with self.assertRaisesRegex(ValueError,'tail not erased'):m.migrate(altered,own,'elite')
        stale=bytearray(src);stale[0]^=1;other=m.pack_backup(bytes(stale),'elite')
        img,_=m.migrate(src,b,'elite')
        self.assertFalse(m.may_retire(img,other,'elite'))
        with self.assertRaisesRegex(ValueError,'lineage mismatch'):m.rollback(img,other,'elite')

    def test_erased_object_preserved(self):
        for missing in (0,4):
            src=fixture(missing=missing);g=m.pack_backup(src,'pro');img,_=m.migrate(src,g,'pro')
            self.assertEqual(m.reader(img),m.legacy_snapshot(src)[1])

    def test_resume_reconstructs_after_every_named_stage(self):
        for active in (0,1):
            src=fixture(active); b=m.pack_backup(src,'elite'); final,ops=m.migrate(src,b,'elite')
            im=bytearray(src)
            for op in ops:
                for n in sorted({0,1,len(op.data)//2,len(op.data)-1,len(op.data)}):
                    partial=bytearray(im)
                    m.write(partial,op,n)
                    self.assertEqual(m.resume(bytes(partial),b,'elite')[0],final)
                m.write(im,op)
            self.assertEqual(m.resume(final,b,'elite')[1],())
            corrupted=bytearray(src);corrupted[m.SET[1]+4094]=5
            with self.assertRaisesRegex(ValueError,'unknown interrupted journal state'):
                m.resume(bytes(corrupted),b,'elite')

    def test_cli_exclusive_atomic_file(self):
        with tempfile.TemporaryDirectory() as d:
            d=Path(d);flash=d/'old-fake.bin';flash.write_bytes(fixture())
            js=d/'backup.json';new=d/'migrated-fake.bin';old=d/'restored-fake.bin'
            cmd=[sys.executable,str(Path(m.__file__).resolve())]
            for how,args in [('backup',[flash,js]),('migrate',[flash,new,'--verified-backup',js]),
                             ('rollback',[new,old,'--verified-backup',js])]:
                subprocess.run([*cmd,how,'--board','pro',*map(str,args)],check=True)
            self.assertEqual(old.read_bytes(),flash.read_bytes())
            p=subprocess.run([*cmd,'backup','--board','pro',str(flash),str(js)],capture_output=True)
            self.assertNotEqual(p.returncode,0)

if __name__=='__main__':unittest.main()
