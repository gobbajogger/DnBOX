#!/usr/bin/env python3
"""Fresh source-bound procedural-model tests. No whole-SuperJ6 or fit claim."""
import argparse,hashlib,json,math,pathlib,shlex,struct,subprocess
SOURCES=['sj6_math_model.c','sj6_math_model.h','sj6_sine_table.inc','model-unit-test.c','model-boundary-regression.c','model-span-test.c','model-render.c','model-build-span-test.c','model-state-span-test.c']
FROZEN_CONTENT='72b55c462402bedc464b90912c7705a9948371350d902be7ace4183618ff798f'
def sha(b):return hashlib.sha256(b).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=pathlib.Path,required=True);p.add_argument('--recovery-root',type=pathlib.Path);a=p.parse_args();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);src=pathlib.Path(__file__).resolve().parent;allowed={};commands=[];results=[]
 for n in SOURCES+['run_qualify.py']:
  f=out/n;b=(src/n).read_bytes();f.write_bytes(b);allowed[f]=sha(b)
 def verify():
  for f,h in allowed.items():
   if sha(f.read_bytes())!=h:raise ValueError('Changed input '+str(f))
 def build(name):
  cmd=['cc','-std=c11','-O2','-ffp-contract=off','-fno-fast-math','-I'+str(out),'-MMD','-MF',str(out/(name+'.d')),str(out/'sj6_math_model.c'),str(out/(name+'.c')),'-lm','-o',str(out/name)]
  # Compile each source separately so every actual dependency has its own receipt.
  objs=[]
  for i,f in enumerate(['sj6_math_model.c',name+'.c']):
   obj=out/(name+f'-{i}.o');dep=out/(name+f'-{i}.d');c=cmd[:7]+['-MMD','-MF',str(dep),'-c',str(out/f),'-o',str(obj)];verify();subprocess.run(c,check=True);commands.append(c)
   for n in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
    if pathlib.Path(n).resolve() not in allowed:raise ValueError('Unlisted dependency '+n)
   objs.append(obj)
  link=['cc',*[str(x) for x in objs],'-lm','-o',str(out/name)];subprocess.run(link,check=True);commands.append(link);verify();return out/name
 for name in ['model-unit-test','model-boundary-regression','model-span-test','model-build-span-test','model-state-span-test']:
  exe=build(name);h=sha(exe.read_bytes());r=subprocess.run([str(exe)],capture_output=True,text=True);verify();assert sha(exe.read_bytes())==h;(out/(name+'.log')).write_text(r.stdout+r.stderr);results.append({'test':name,'exit':r.returncode,'executable_sha256':h,'stdout':r.stdout,'stderr':r.stderr});assert r.returncode==0
 rendered=[]
 if a.recovery_root:
  root=a.recovery_root.resolve();freeze=root/'superj6-qualification/FREEZE-MATH-R2.private.json';b=freeze.read_bytes();d=json.loads(b)
  assert sha(json.dumps(d['files'],sort_keys=True,separators=(',',':')).encode())==FROZEN_CONTENT
  frozen={e['path']:e for e in d['files']};allowed[freeze]=sha(b);exe=build('model-render');eh=sha(exe.read_bytes())
  for tag,base in [('compact512','superj6-qualification/private/math-renderer'),('full2048','superj6-qualification/private/math-renderer/full2048')]:
   mp=base+'/models/MANIFEST.private.json';mb=(root/mp).read_bytes();assert sha(mb)==frozen[mp]['sha256'];allowed[root/mp]=sha(mb)
   for row in json.loads(mb)['models']:
    name=row['name'];rel=base+'/models/'+name+'.sj6m.private';b=(root/rel).read_bytes();assert sha(b)==frozen[rel]['sha256']==row['model_sha256'];model=out/'models'/tag/(name+'.sj6m');model.parent.mkdir(parents=True,exist_ok=True);model.write_bytes(b);allowed[model]=sha(b)
    expected=frozen[base+'/renders/'+name+'.f32.private'];target=out/'renders'/tag/(name+'.f32');target.parent.mkdir(parents=True,exist_ok=True);verify();r=subprocess.run([str(exe),str(model),str(target)],capture_output=True,text=True);verify();assert sha(exe.read_bytes())==eh;assert r.returncode==0
    data=target.read_bytes();assert len(data)==row['frames']*4==expected['bytes'];assert sha(data)==expected['sha256'];assert all(math.isfinite(x[0]) for x in struct.iter_unpack('<f',data));rendered.append({'set':tag,'name':name,'frames':row['frames'],'bytes':len(data),'sha256':sha(data),'model_sha256':allowed[model],'exact_frozen_output':True,'log':r.stdout});print(tag,name,'PASS',flush=True)
 verify();receipt={'scope':__doc__,'tests':results,'renders':rendered,'commands':commands,'inputs':{str(k):v for k,v in allowed.items()},'frozen_content_sha256':FROZEN_CONTENT if a.recovery_root else None};(out/'RESULT.json').write_text(json.dumps(receipt,indent=2)+'\n');print('PASS',len(results),'synthetic suites and',len(rendered),'frozen model vectors')
if __name__=='__main__':main()
