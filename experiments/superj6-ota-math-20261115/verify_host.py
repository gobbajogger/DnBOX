#!/usr/bin/env python3
"""Pinned fixture-free host checks. No original app, PCM, SDK or native execution."""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess,sys,datetime
def fail(c,m):
 if not c:raise ValueError(m)
def sha(b):return hashlib.sha256(b).hexdigest()
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--out',required=True);ap.add_argument('--manifest-sha256',required=True);a=ap.parse_args()
 R=Path(__file__).resolve().parent;mf=(R/'SOURCE-MANIFEST.json').read_bytes();fail(sha(mf)==a.manifest_sha256,'manifest pin')
 files=json.loads(mf)['files'];expected={e['path']for e in files}|{'SOURCE-MANIFEST.json'}
 actual={str(f.relative_to(R))for f in R.rglob('*')if f.is_file()or f.is_symlink()};fail(actual==expected,'unlisted/missing input')
 for e in files:
  p=R/e['path'];b=p.read_bytes();fail(not p.is_symlink()and len(b)==e['bytes']and sha(b)==e['sha256']and hashlib.sha1(b'blob '+str(len(b)).encode()+b'\0'+b).hexdigest()==e['git_blob'],'input:'+e['path'])
 out=Path(a.out).resolve();fail(not out.is_relative_to(R),'output within source');fail(not out.exists(),'output exists');out.mkdir()
 stage=out/'stage';stage.mkdir()
 for name in expected:
  d=stage/name;d.parent.mkdir(parents=True,exist_ok=True);d.write_bytes((R/name).read_bytes());fail(d.read_bytes()==(R/name).read_bytes(),'staged bytes')
 (stage/'build').mkdir()
 env={'PATH':os.defpath,'LC_ALL':'C','LANG':'C','PYTHONDONTWRITEBYTECODE':'1','TMPDIR':str(out)}
 commands=[
  [sys.executable,str(stage/'run_host.py')],
  ['cc','-std=c11','-O2','-ffp-contract=off','-fno-fast-math','-shared','-fPIC','-I'+str(stage/'math'),str(stage/'math/numeric_boundary.c'),'-o',str(stage/'build/libnumeric.so')],
  [sys.executable,str(stage/'tests/conversion_oracle.py'),str(stage/'build/libnumeric.so')],
 ]
 for i,cmd in enumerate(commands):
  p=subprocess.run(cmd,capture_output=True,env=env);(out/f'command-{i}.stdout').write_bytes(p.stdout);(out/f'command-{i}.stderr').write_bytes(p.stderr);fail(p.returncode==0,'command failed:'+p.stderr.decode(errors='replace'))
 # Deterministic generation is checked in the disposable copy, never source.
 p=subprocess.run([sys.executable,str(stage/'derive_integration.py')],capture_output=True,env=env);(out/'derive.stdout').write_bytes(p.stdout);(out/'derive.stderr').write_bytes(p.stderr);fail(p.returncode==0,'derivation failed')
 for name in ['candidate/OriginalOTAFilter.h','baseline/OriginalOTA-FMA-only.h','math/numeric_boundary.c','DERIVATION.json']:
  fail((stage/name).read_bytes()==(R/name).read_bytes(),'derivation drift:'+name)
 fail({str(f.relative_to(R))for f in R.rglob('*')if f.is_file()or f.is_symlink()}==expected,'source closure changed')
 for e in files:fail(sha((R/e['path']).read_bytes())==e['sha256'],'source changed')
 result={'completed_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'manifest_sha256':a.manifest_sha256,'fixture_free_host_checks':True,'source_unchanged':True,'scope':'Pure source/math OTA controlled-finite RNE only. No full sampler/original/target execution, firmware fit, CPU or device qualification. See retained per-suite logs: defaultLSan ptrace failures are not leaks-disabled passes.'}
 (out/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result))
if __name__=='__main__':main()
