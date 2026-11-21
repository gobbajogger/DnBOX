#!/usr/bin/env python3
"""Stage real baseline/current morph and reader, verify exact behavior and copy calls."""
import argparse,hashlib,json,os,shlex,subprocess,shutil
from pathlib import Path
BASELINE='85c30ba0b6022c48a6f63e6503279b122226ea83096d657f8030112a72741921'
RETAINED='042a90468216b11a05de1afd5305de2ee8493becfb73e39a82fc5840fa924529'
PACK='a5cf8182b8bc9ca0a1b034a6b63f55d2cdc1087df38d6b1433da4fb56e5eccd5'
def sha(b):return hashlib.sha256(b).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--retained-build',type=Path,required=True);p.add_argument('--pack',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);repo=Path(__file__).resolve().parents[2];frozen=a.retained_build.resolve();allowed={}
 def stage(name,data,pin=None):
  if pin and sha(data)!=pin:raise ValueError('pinned input mismatch: '+name)
  path=out/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(data);allowed[path.resolve()]=sha(data);return path
 closure=frozen/'closure';morph=closure/'scheduler/compact-reader/compact_morph.c'
 metadata=(frozen/'TESTED-SOURCES.json').read_bytes();stage('retained-tested-sources.json',metadata,RETAINED);pins=json.loads(metadata)
 depbytes=(frozen/'compile-016.d').read_bytes();stage('retained-compile.d',depbytes)
 stage('baseline/compact_morph.c',morph.read_bytes(),BASELINE)
 # Enumerate exactly the already-tested morph compile's local header closure.
 deps=shlex.split(depbytes.decode().replace('\\\n',' ').split(':',1)[1])
 for name in deps:
  src=Path(name).resolve();relative=src.relative_to(closure)
  if src==morph:continue
  stage('closure/'+str(relative),src.read_bytes(),pins[str(relative)])
 current=repo/'firmware/src/dsp/synthone/scheduler/compact-reader'
 for name in ['compact.c','compact.h','compact_morph.c','compact_morph.h']:
  stage('closure/scheduler/compact-reader/'+name,(current/name).read_bytes())
 stage('test.c',(repo/'tests/host/synthone_morph_bitcopy_test.c').read_bytes());stage('pack.bin',a.pack.read_bytes(),PACK);stage('runner.py',Path(__file__).read_bytes())
 includes=['scheduler','scheduler/compact-reader','experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe','experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe/lib/dr_wav','experiment/audiokit-4.9.2/AudioKit/Core/SoundpipeExtension']
 flags=['-std=gnu99','-O2','-fno-builtin','-ffp-contract=off','-fno-fast-math','-DNO_LIBSNDFILE']+['-I'+str(out/'closure'/x)for x in includes]
 compiler=Path(shutil.which('cc')).resolve();compiler_hash=sha(compiler.read_bytes());compiler_version=subprocess.check_output([str(compiler),'--version']).decode()
 commands=[]
 def verify():
  if sha(compiler.read_bytes())!=compiler_hash:raise ValueError('compiler changed')
  for f,h in allowed.items():
   if sha(f.read_bytes())!=h:raise ValueError('staged input changed: '+str(f))
 def compile(source,obj,extra=[]):
  dep=out/(obj+'.d');cmd=[str(compiler),*flags,*extra,'-MMD','-MF',str(dep),'-c',str(out/source),'-o',str(out/obj)];commands.append(cmd);verify();subprocess.run(cmd,check=True)
  for name in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
   if Path(name).resolve() not in allowed:raise ValueError('unlisted compiled dependency: '+name)
  verify()
 compile('baseline/compact_morph.c','baseline.o',['-Ds1c_runtime_morph_compute=s1c_baseline_morph_compute'])
 compile('closure/scheduler/compact-reader/compact_morph.c','candidate.o')
 compile('closure/scheduler/compact-reader/compact.c','reader.o')
 compile('test.c','test.o')
 cmd=[str(compiler),str(out/'test.o'),str(out/'baseline.o'),str(out/'candidate.o'),str(out/'reader.o'),'-Wl,--wrap=memcpy','-o',str(out/'test')];commands.append(cmd);subprocess.run(cmd,check=True);verify()
 exe=sha((out/'test').read_bytes());r=subprocess.run([str(out/'test'),str(out/'pack.bin')],capture_output=True);(out/'run.log').write_bytes(r.stdout+r.stderr);verify();assert sha((out/'test').read_bytes())==exe
 result={'scope':__doc__,'compiler':str(compiler),'compiler_sha256':compiler_hash,'compiler_version':compiler_version,'retained_manifest_sha256':RETAINED,'exit':r.returncode,'baseline_morph_sha256':BASELINE,'inputs':{str(f.relative_to(out)):h for f,h in allowed.items()},'commands':commands,'executable_sha256':exe,'log_sha256':sha(r.stdout+r.stderr),'full_audio_native_timing':'NOT_RUN'}
 (out/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n');print((r.stdout+r.stderr).decode());raise SystemExit(r.returncode)
if __name__=='__main__':main()
