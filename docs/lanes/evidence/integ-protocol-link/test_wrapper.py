"""Control-flow/metadata fixtures only; stubbed compiler, not native qualification."""
import hashlib, json, os, pathlib, shutil, subprocess, tempfile, unittest
HERE = pathlib.Path(__file__).resolve().parent
ROOTS = ['dnr2_init','dnr2_changed','dnr2_tick','dnr2_handle','dnr2_decode','dnr2_encode_command','dnr2_encode_reply','dnr2_receive']
BUILD = '''import os
from pathlib import Path
BOARD='smk37'; MODEL=os.environ['MODEL']; OUT=Path.cwd()/'build/smk37-elite'
def tc(*args,**kwargs):
    OUT.joinpath('x0x.elf').write_bytes(b'probe')
    Path(args[args.index('-Map')+1]).write_text('map')
def main():
    mode=os.environ['FIXTURE_MODE']
    if mode=='primary': raise RuntimeError('ORIGINAL_SETUP_FAILURE')
    if mode=='return': return 7
    tc('pi32v2/bin/ld','-o',OUT/'x0x.elf')
    return 0
'''
class Wrapper(unittest.TestCase):
    def run_case(self, mode):
        with tempfile.TemporaryDirectory() as tmp:
            base=pathlib.Path(tmp); root=base/'repo'; helper=base/'helper'; root.mkdir(); helper.mkdir()
            (root/'tools').mkdir(); (root/'tools/build.py').write_text(BUILD)
            def git(*args): return subprocess.check_output(['git',*args],cwd=root,stderr=subprocess.DEVNULL).decode().strip()
            git('init','-q'); git('config','user.name','Fixture'); git('config','user.email','fixture@example.invalid'); git('add','.'); git('commit','-qm','fixture')
            baseline=b'baseline'; out=root/'build/smk37-elite'; out.mkdir(parents=True)
            (out/'x0x.elf').write_bytes(baseline); (out/'x0x.map').write_text('baseline-map')
            manifest={'source':git('rev-parse','HEAD'),'build_py_sha256':hashlib.sha256(BUILD.encode()).hexdigest(),'baseline_elf_sha256':{'elite':hashlib.sha256(baseline).hexdigest()},'objects':[],'entry_points':ROOTS}
            for name in ['core.o','wire.o']:
                data=name.encode(); (helper/name).write_bytes(data); manifest['objects'].append({'name':name,'sha256':hashlib.sha256(data).hexdigest()})
            (helper/'inputs.json').write_text(json.dumps(manifest)); shutil.copyfile(HERE/'link_probe.py',helper/'link_probe.py')
            tc=base/'toolchain'; (tc/'pi32v2/bin').mkdir(parents=True); (tc/'pi32v2/bin/clang').write_bytes(b'fixture-compiler')
            sdk=base/'sdk'; (sdk/'cpu/wl82/tools').mkdir(parents=True); (sdk/'cpu/wl82/tools/uboot.boot').write_bytes(b'fixture-sdk')
            bin=base/'bin'; bin.mkdir(); readelf=bin/'readelf'; names=ROOTS[:-1] if mode=='missing' else ROOTS
            readelf.write_text('#!/bin/sh\n'+''.join("printf '%s\\n' '1: 00000000 4 FUNC GLOBAL DEFAULT 1 "+x+"'\n" for x in names)); readelf.chmod(0o755)
            env=dict(os.environ,MODEL='elite',BOARD='smk37',X0X_DEV_PADCAL='1',FIXTURE_MODE=mode,JIELI_TOOLCHAIN=str(tc),AC79_SDK=str(sdk),PATH=str(bin)+os.pathsep+os.environ['PATH'])
            result=subprocess.run(['python3',str(helper/'link_probe.py')],cwd=root,env=env,capture_output=True,text=True)
            receipt=json.loads((root/'build/protocol-probe-elite/receipt.json').read_text())
            self.assertEqual((root/'build/protocol-probe-elite/baseline.elf').read_bytes(),baseline)
            return result,receipt
    def test_success(self):
        r,d=self.run_case('ok'); self.assertEqual(r.returncode,0,r.stderr); self.assertEqual(d['status'],'LINKED_DIAGNOSTIC'); self.assertFalse(d['missing_retained_symbols'])
    def test_nonzero_return(self):
        r,d=self.run_case('return'); self.assertNotEqual(r.returncode,0); self.assertEqual(d['build_return'],7); self.assertEqual(d['status'],'NOT_QUALIFIED'); self.assertFalse(d['build_completed'])
    def test_primary_error(self):
        r,d=self.run_case('primary'); self.assertNotEqual(r.returncode,0); self.assertIn('ORIGINAL_SETUP_FAILURE',r.stderr); self.assertIn('ORIGINAL_SETUP_FAILURE',d['primary_error']); self.assertFalse(d['build_completed'])
    def test_missing_symbol(self):
        r,d=self.run_case('missing'); self.assertNotEqual(r.returncode,0); self.assertEqual(d['status'],'NOT_QUALIFIED'); self.assertEqual(d['missing_retained_symbols'],['dnr2_receive'])
if __name__=='__main__': unittest.main()
