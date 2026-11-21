#!/usr/bin/env python3
"""Stage exact original scalar and current batch reader; no runtime activation."""
import argparse,hashlib,json,pathlib,shlex,subprocess,os
ORIGINAL={'compact.c':'a5e63c119d98b6708d6809c7bec093104a9299aac6b95fd3ac51fb90c880e7b5','compact.h':'6899920139862016acbdec4a88949e376972b3529fd29617af1a5cf6aae521b4'}
PACK='a5cf8182b8bc9ca0a1b034a6b63f55d2cdc1087df38d6b1433da4fb56e5eccd5'
def sha(b):return hashlib.sha256(b).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--original-reader',type=pathlib.Path,required=True);p.add_argument('--pack',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--sanitize',action='store_true');a=p.parse_args();out=a.output.resolve();out.mkdir(exist_ok=False,parents=True)
 repo=pathlib.Path(__file__).resolve().parents[2];allowed={}
 def stage(name,data,expected=None):
  if expected and sha(data)!=expected:raise ValueError('Pinned input mismatch: '+name)
  f=out/name;f.parent.mkdir(exist_ok=True,parents=True);f.write_bytes(data);allowed[f.resolve()]=sha(data);return f
 for n,h in ORIGINAL.items():stage('original/'+n,(a.original_reader/n).read_bytes(),h)
 for n in ORIGINAL:stage('candidate/'+n,(repo/'firmware/src/dsp/synthone/scheduler/compact-reader'/n).read_bytes())
 stage('test.c',(repo/'tests/host/synthone_compact_quad_test.c').read_bytes());stage('compact.bin',a.pack.read_bytes(),PACK);stage('runner.py',pathlib.Path(__file__).read_bytes())
 def verify():
  for f,h in allowed.items():
   if sha(f.read_bytes())!=h:raise ValueError('Staged input changed: '+str(f))
 def deps(path):
  for n in shlex.split(path.read_text().replace('\\\n',' ').split(':',1)[1]):
   if pathlib.Path(n).resolve() not in allowed:raise ValueError('Unlisted dependency: '+n)
  verify()
 flags=['-O2','-std=c11']+(['-fsanitize=address,undefined','-fno-omit-frame-pointer'] if a.sanitize else [])
 c0=['cc',*flags,'-Ds1c_read=s1c_original_read','-Ds1c_bind=s1c_original_bind','-Ds1c_binding_valid=s1c_original_binding_valid','-MMD','-MF',str(out/'original.d'),'-c',str(out/'original/compact.c'),'-o',str(out/'original.o')]
 c1=['cc',*flags,'-finstrument-functions','-I'+str(out/'candidate'),'-MMD','-MF',str(out/'test.d'),str(out/'test.c'),str(out/'original.o'),'-o',str(out/'test')]
 verify();subprocess.run(c0,check=True);deps(out/'original.d');subprocess.run(c1,check=True);deps(out/'test.d');exe=sha((out/'test').read_bytes());r=subprocess.run([str(out/'test'),str(out/'compact.bin'),'2'],capture_output=True,text=True);(out/'run.log').write_text(r.stdout+r.stderr);verify()
 if sha((out/'test').read_bytes())!=exe:raise ValueError('Executable changed')
 receipt={'scope':__doc__,'exit':r.returncode,'sanitizers':a.sanitize,'asan_options':os.environ.get('ASAN_OPTIONS',''),'lsan_options':os.environ.get('LSAN_OPTIONS',''),'original_source':'56079bbd compact reader preserved at6a7','inputs':{str(k.relative_to(out)):v for k,v in allowed.items()},'commands':[c0,c1],'executable_sha256':exe,'log_sha256':sha((out/'run.log').read_bytes())};(out/'RESULT.json').write_text(json.dumps(receipt,indent=2)+'\n');print(r.stdout+r.stderr);raise SystemExit(r.returncode)
if __name__=='__main__':main()
