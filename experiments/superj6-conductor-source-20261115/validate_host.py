"""Build only this new controller and own host tests; no original/target code."""
from pathlib import Path
import argparse,subprocess,hashlib,json,datetime,os,shutil,sys
p=argparse.ArgumentParser();p.add_argument('--out',required=True);p.add_argument('--source-pin',required=True);a=p.parse_args();root=Path(__file__).resolve().parent;out=Path(a.out).resolve()
mf=root/'SOURCE-MANIFEST.json'
b=mf.read_bytes()
if hashlib.sha256(b).hexdigest()!=a.source_pin:raise SystemExit('source manifest pin mismatch')
m=json.loads(b);expected={'SOURCE-MANIFEST.json'}
for row in m['files']:
 rel=Path(row['path'])
 if rel.is_absolute() or '..' in rel.parts or row['path'] in expected:raise SystemExit('invalid manifest path')
 expected.add(row['path'])
 f=root/rel
 if f.is_symlink() or not f.is_file():raise SystemExit('invalid source file')
 content=f.read_bytes()
 if len(content)!=row['bytes'] or hashlib.sha256(content).hexdigest()!=row['sha256']:raise SystemExit('source bytes mismatch')
if any(f.is_symlink()for f in root.rglob('*')):raise SystemExit('source symlink forbidden')
actual={str(f.relative_to(root))for f in root.rglob('*')if f.is_file()}
if actual!=expected:raise SystemExit('source closure mismatch')
if out==root or root in out.parents or out in root.parents or out.exists():raise SystemExit('output must be new and disjoint')
out.mkdir();rows=[]
def call(name,cmd,env=None):
 r=subprocess.run(cmd,cwd=root,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
 b=r.stdout;(out/(name+'.log')).write_bytes(b);rows.append({'name':name,'command':cmd,'exit':r.returncode,'log_sha256':hashlib.sha256(b).hexdigest()});return r
compiler=shutil.which('g++');assert compiler
r=call('compiler',[compiler,'--version']);assert r.returncode==0
base=[compiler,'-std=c++11','-pedantic-errors','-fno-exceptions','-Wall','-Wextra','-Werror','-ffp-contract=off','-fno-fast-math']
env=dict(os.environ,PYTHONDONTWRITEBYTECODE='1')
r=call('python',[sys.executable,'-m','unittest','discover','-v'],env);assert r.returncode==0
for variant,flags in [('o2',['-O2']),('os',['-Os']),('sanitize',['-O1','-g','-fsanitize=address,undefined,float-cast-overflow','-fno-sanitize-recover=all'])]:
 for test in ['test_conductor','test_restore','test_deferred','test_lfo_laws','test_entropy','cross_probe','entropy_probe','test_tick_prefix','test_tick_time','test_lfo_apply','test_vibrato4','test_cutoff_executor','test_signed_zero','test_ordered_tick','test_lifecycle','prefix_equivalence_probe','test_preset_executor','test_defaults','test_pwm_postpass','test_timing_exact']:
  label=variant+'-'+test;exe=out/label
  r=call(label+'-build',base+flags+[str(root/'conductor.cpp'),str(root/'lfo_laws.cpp'),str(root/'entropy_table.cpp'),str(root/'tick_prefix.cpp'),str(root/'tick_time.cpp'),str(root/'lfo_apply.cpp'),str(root/'vibrato4.cpp'),str(root/'cutoff_executor.cpp'),str(root/'ordered_tick.cpp'),str(root/'lifecycle.cpp'),str(root/'preset_executor.cpp'),str(root/'prefix_reference_testonly.cpp'),str(root/(test+'.cpp')),'-o',str(exe)]);assert r.returncode==0,label
  before=hashlib.sha256(exe.read_bytes()).hexdigest();r=call(label,[str(exe)],env)
  if variant=='sanitize' and r.returncode!=0:
   text=r.stdout.decode(errors='replace')
   assert 'LeakSanitizer does not work under ptrace' in text and 'runtime error:' not in text and 'ERROR: AddressSanitizer' not in text,(label,text)
   rows[-1]['qualification']='DEFAULT_LSAN_PTRACE_FAILURE_NOT_LEAK_PASS'
   r=call(label+'-leaks-disabled',[str(exe)],dict(env,ASAN_OPTIONS='detect_leaks=0'))
   rows[-1]['qualification']='LEAK_DETECTION_DISABLED'
   assert hashlib.sha256(exe.read_bytes()).hexdigest()==before
  assert r.returncode==0,label
  rows[-1]['binary_sha256']=before
  if test in ('cross_probe','entropy_probe'):
   data=out/(label+'.records');data.write_bytes(r.stdout)
   r=call(label+'-compare',[sys.executable,str(root/('cross_check.py'if test=='cross_probe'else'entropy_check.py')),str(data)],env);assert r.returncode==0
result={'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'status':'PASS_BOUNDED_OWN_HOST_TESTS','scope':'control/state/command source only; no original platform math, native, real DSP, full LFO or product acceptance','source_manifest_sha256':a.source_pin,'compiler_path':compiler,'compiler_sha256':hashlib.sha256(Path(compiler).read_bytes()).hexdigest(),'runs':rows}
(out/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'status':result['status'],'runs':len(rows),'result':str(out/'RESULT.json')}))
