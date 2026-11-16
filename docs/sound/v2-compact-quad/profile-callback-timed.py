"""Callback-only gprof diagnostic; probe overhead invalidates timing qualification."""
import json,hashlib,subprocess
from pathlib import Path
base=Path('sound-v2-evidence/callback-6a7-pilot').resolve();out=Path('sound-v2-evidence/callback-profile-timed').resolve();out.mkdir(exist_ok=False)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
inputs=json.loads((base/'CALLBACK-INPUTS.json').read_bytes());allowed={base/n:h for n,h in inputs.items()}
def verify():
 for p,h in allowed.items():assert sha(p)==h,p
verify()
# Execute only the already source-bound captured helper, not a mutable live runner.
helper=base/'orchestration-run_synthone_factory.py';namespace={'__name__':'profile_factory_helpers','__file__':str(helper)};exec(compile(helper.read_bytes(),str(helper),'exec'),namespace)
source=(base/'callback-bench.cpp').read_text();old='int main(int argc, char **argv) {';assert source.count(old)==1
source=source.replace(old,'extern "C" void moncontrol(int);\n'+old+'\n    moncontrol(0);')
a='            const uint64_t wall_start = clock_ns(CLOCK_MONOTONIC);';b='            const uint64_t cpu = clock_ns(CLOCK_THREAD_CPUTIME_ID) - cpu_start;';assert source.count(a)==source.count(b)==1
source=source.replace(a,'            moncontrol(1);\n'+a).replace(b,'            moncontrol(0);\n'+b)
bench=out/'callback-profile.cpp';bench.write_text(source);allowed[bench]=sha(bench)
commands=[]
for f in ['commands.json','factory-build-commands.json','CALLBACK-COMMANDS.json']:commands+=json.loads((base/f).read_bytes())
compiles={c[c.index('-o')+1]:c for c in commands if '-c' in c};link=commands[-1];required=[x for x in link if x.endswith('.o')];assert len(set(required))==len(required);replay=[]
for index,obj in enumerate(required):
 c=list(compiles[obj]);c.insert(1,'-pg');c[c.index('-o')+1]=str(out/Path(obj).name)
 if c[c.index('-c')+1]==str(base/'callback-bench.cpp'):c[c.index('-c')+1]=str(bench)
 dep=out/f'{index}.d';c[c.index('-MF')+1]=str(dep);replay.append(c);subprocess.run(c,check=True);namespace['verify_dependencies'](dep,allowed)
link=[str(out/Path(x).name) if x in required else x for x in link];link.insert(1,'-pg');link[link.index('-o')+1]=str(out/'callback-profile');replay.append(link);subprocess.run(link,check=True);verify()
(out/'PROFILE-COMMANDS.json').write_text(json.dumps(replay,indent=2)+'\n');binary_hash=sha(out/'callback-profile')
cmd=[str(out/'callback-profile'),str(base/'factory-inputs/public-morphs.f32'),str(base/'factory-inputs/compact.bin'),str(base/'commands/preset-commands.bin'),str(out/'trace.csv'),str(out/'programs.jsonl'),'8','0'];r=subprocess.run(cmd,cwd=out,capture_output=True,text=True);(out/'run.log').write_text(r.stdout+r.stderr);verify();assert sha(out/'callback-profile')==binary_hash
r2=subprocess.run(['gprof','-b',str(out/'callback-profile'),str(out/'gmon.out')],capture_output=True,text=True,check=True);(out/'gprof.txt').write_text(r2.stdout)
receipt={'scope':__doc__,'qualification':'NOT_QUALIFIED','runtime_pin':'6a7ee3abec7359b5b03544b672d4ab236f1c2ba6','exit':r.returncode,'inputs':{str(p):h for p,h in allowed.items()},'binary_sha256':binary_hash,'run_log_sha256':sha(out/'run.log'),'gprof_sha256':sha(out/'gprof.txt')};(out/'PROFILE-RESULT.json').write_text(json.dumps(receipt,indent=2)+'\n');print(r.stdout);print(r2.stdout[:5000])
