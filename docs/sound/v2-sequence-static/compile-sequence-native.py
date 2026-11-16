from pathlib import Path
import json,hashlib,subprocess,shutil,shlex,struct
root=Path.cwd();base=root/'sound-v2-evidence/native-pinned/pi32v2.uaosjrsj';recipe=json.loads((base/'RESULT.json').read_bytes())
repo=root/'sound-v2';runtime=repo/'firmware/src/dsp/synthone';out=root/'sound-v2-evidence/native-sequence-final';out.mkdir()
sha=lambda b:hashlib.sha256(b).hexdigest()
for f in json.loads((runtime/'SOURCE-MANIFEST.json').read_bytes())['files']:
 assert sha((runtime/f['path']).read_bytes())==f['sha256'],f['path']
shutil.copytree(runtime,out/'closure');(out/'adapter').mkdir()
for n in ['synthone_c.cpp','synthone_c.h']:shutil.copyfile(repo/'firmware/src/dsp'/n,out/'adapter'/n)
compiler=Path(recipe['units'][0]['command'][0]);assert sha(compiler.read_bytes())==recipe['compiler_sha256']
commands=[]
for unit in recipe['units']:
 if unit['name']=='layout':continue
 cmd=[x.replace(str(base/'closure'),str(out/'closure')).replace(str(base),str(out)).replace(str(root/'synthone-recovery/source-56079bbd/firmware/src/dsp'),str(out/'adapter'))for x in unit['command']]
 commands.append((unit['name'],cmd))
frontbase=root/'sound-v2-evidence/frontend-cabi-tested-v2';frontout=root/'sound-v2-evidence/frontend-native'
for name,cmd in zip(['wrapper','frontend'],json.loads((frontout/'commands.json').read_bytes())):
 cmd=[x.replace(str(frontbase/'closure'),str(out/'closure')).replace(str(frontbase/'adapter-inputs/firmware/src/dsp'),str(out/'adapter')).replace(str(frontout),str(out))for x in cmd];commands.append((name,cmd))
layout=out/'layout.cpp';layout.write_text('#include "source_scheduler.hpp"\nstatic_assert(sizeof(s1scheduler::SourceScheduler)==7972,"owner growth");\nstatic_assert(alignof(s1scheduler::SourceScheduler)==4,"alignment changed");\nchar s1_owner_size[sizeof(s1scheduler::SourceScheduler)];\n')
cmd=list(commands[-2][1]);cmd[cmd.index('-c')+1]=str(layout);cmd[cmd.index('-o')+1]=str(out/'layout.o');cmd[cmd.index('-MF')+1]=str(out/'layout.d');commands.append(('layout',cmd))
allowed={p.resolve():sha(p.read_bytes())for d in [out/'closure',out/'adapter']for p in d.rglob('*')if p.is_file()};allowed[layout]=sha(layout.read_bytes())
sdkroots=set()
for _,cmd in commands:
 for i,x in enumerate(cmd[:-1]):
  if x=='-isystem':sdkroots.add(Path(cmd[i+1]))
for directory in sdkroots:
 for p in directory.rglob('*'):
  if p.is_file():allowed[p.resolve()]=sha(p.read_bytes())
def verify():
 for p,h in allowed.items():assert sha(p.read_bytes())==h,str(p)
def sections(p):
 b=p.read_bytes();assert b[:6]==b'\x7fELF\x01\x01';off=struct.unpack_from('<I',b,32)[0];size,num=struct.unpack_from('<HH',b,46);d=dict(executable=0,readonly=0,writable=0,bss=0)
 for i in range(num):
  _,typ,flags,_,_,n,_,_,_,_=struct.unpack_from('<10I',b,off+i*size)
  if flags&2:d['bss' if typ==8 else 'executable' if flags&4 else 'writable' if flags&1 else 'readonly']+=n
 return d
(out/'compiler-temp').mkdir();env=dict(PATH='/bin:/usr/bin',LC_ALL='C',LANG='C',TMPDIR=str(out/'compiler-temp'));results=[]
for name,cmd in commands:
 verify();r=subprocess.run(cmd,capture_output=True,text=True,env=env);(out/(name+'.log')).write_text(r.stdout+r.stderr)
 print(name,r.returncode,flush=True);assert r.returncode==0,name
 dep=Path(cmd[cmd.index('-MF')+1]);dependencies=[]
 for x in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
  p=Path(x).resolve();assert p in allowed,str(p);assert sha(p.read_bytes())==allowed[p];dependencies.append({'path':str(p),'sha256':allowed[p]})
 obj=Path(cmd[cmd.index('-o')+1]);results.append({'name':name,'command':cmd,'object_sha256':sha(obj.read_bytes()),'allocated_sections':sections(obj),'dependencies':dependencies})
verify();(out/'RESULT.json').write_text(json.dumps({'scope':'31 native runtime/wrapper/frontend object compiles plus layout only, no application link or fit','compiler_sha256':recipe['compiler_sha256'],'owner_bytes':7972,'owner_alignment':4,'units':results},indent=2)+'\n')
print('PASS native component compilation and unchanged7972/4 owner; no app link')
