from pathlib import Path
import json,hashlib,subprocess,shlex
root=Path(__file__).resolve().parent;base=root.parent/'compact-quad-final';allowed={base/k:v for k,v in json.loads((base/'RESULT.json').read_text())['inputs'].items()};allowed[root/'bench.c']=hashlib.sha256((root/'bench.c').read_bytes()).hexdigest();allowed[Path(__file__).resolve()]=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
def verify():
 for f,h in allowed.items():assert hashlib.sha256(f.read_bytes()).hexdigest()==h,f
verify();commands=[]
for name,source,flags in [('original',base/'original/compact.c',['-Ds1c_read=s1c_original_read','-Ds1c_bind=s1c_original_bind','-Ds1c_binding_valid=s1c_original_binding_valid']),('quad',base/'candidate/compact.c',[])]:
 cmd=['cc','-O2','-std=c11',*flags,'-MMD','-MF',str(root/(name+'-fresh.d')),'-c',str(source),'-o',str(root/(name+'-fresh.o'))];commands.append(cmd);subprocess.run(cmd,check=True)
 for n in shlex.split((root/(name+'-fresh.d')).read_text().replace('\\\n',' ').split(':',1)[1]):assert Path(n).resolve() in allowed,n
cmd=['cc','-O2','-std=c11','-I'+str(base/'candidate'),'-MMD','-MF',str(root/'bench-fresh.d'),str(root/'bench.c'),str(root/'original-fresh.o'),str(root/'quad-fresh.o'),'-o',str(root/'bench-fresh')];commands.append(cmd);subprocess.run(cmd,check=True)
for n in shlex.split((root/'bench-fresh.d').read_text().replace('\\\n',' ').split(':',1)[1]):assert Path(n).resolve() in allowed,n
verify();exe=hashlib.sha256((root/'bench-fresh').read_bytes()).hexdigest();r=subprocess.run([str(root/'bench-fresh'),str(base/'compact.bin')],capture_output=True,text=True);verify();assert hashlib.sha256((root/'bench-fresh').read_bytes()).hexdigest()==exe;(root/'fresh.jsonl').write_text(r.stdout);rows=[json.loads(x) for x in r.stdout.splitlines()];assert r.returncode==0 and len(rows)==4 and len({x['sum'] for x in rows})==1
receipt={'scope':'Diagnostic batched word throughput only, not synth callback/native qualification; all four ABBA runs retained','inputs':{str(k):v for k,v in allowed.items()},'commands':commands,'executable_sha256':exe,'runs':rows};(root/'RESULT.json').write_text(json.dumps(receipt,indent=2)+'\n');print(r.stdout)
