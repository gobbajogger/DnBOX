#!/usr/bin/env python3
"""Exploratory ABBA same-host measurement, not hard realtime qualification."""
import pathlib,hashlib,json,subprocess,importlib.util
root=pathlib.Path(__file__).resolve().parent
roots={'A':root/'callback-a803-pilot','B':root/'callback-bitcopy-pilot'}
output=root/'callback-bitcopy-abba';output.mkdir(exist_ok=False)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
inputs={k:json.loads((p/'CALLBACK-INPUTS.json').read_bytes()) for k,p in roots.items()}
assert inputs['A'].keys()==inputs['B'].keys()
assert {k for k in inputs['A'] if inputs['A'][k]!=inputs['B'][k]}=={'closure/scheduler/compact-reader/compact_morph.c'}
exe={k:sha(p/'callback-bench') for k,p in roots.items()}
assert exe['A']==json.loads((roots['A']/'CALLBACK-RESULT.json').read_bytes())['executable_sha256']
assert exe['B']==json.loads((roots['B']/'BUILD-IDENTITY.json').read_bytes())['executable_sha256']
def verify():
 for k,p in roots.items():
  assert sha(p/'callback-bench')==exe[k]
  for f,h in inputs[k].items():assert sha(p/f)==h,(k,f)
verify()
runner=(roots['B']/'callback-runner.py').read_bytes();ns={'__name__':'paired_callback_helpers'};exec(compile(runner,'<verified callback runner>','exec'),ns)
results=[]
for index,k in enumerate('ABBA'):
 verify();p=roots[k];out=output/f'{index}-{k}';out.mkdir()
 cmd=[str(p/'callback-bench'),str(p/'factory-inputs/public-morphs.f32'),str(p/'factory-inputs/compact.bin'),str(p/'commands/preset-commands.bin'),str(out/'trace.csv'),str(out/'programs.jsonl'),'8','0']
 r=subprocess.run(cmd,capture_output=True,text=True);(out/'run.log').write_text(r.stdout+r.stderr);verify()
 s=ns['summarize'](out/'trace.csv',8);progs=[json.loads(x) for x in (out/'programs.jsonl').read_text().splitlines()];assert len(progs)==8
 finite=sum(x['nonfinite_words'] for x in progs);assert r.returncode==int(bool(finite or s['wall_misses'] or s['cpu_misses']))
 result={'order':index,'variant':k,'executable_sha256':exe[k],'benchmark_tracer_pid':ns['parse_benchmark_tracer'](r.stderr),'summary':s,'nonfinite_words':finite,'files':{x.name:sha(x) for x in out.iterdir()}}
 results.append(result);(out/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result),flush=True)
(output/'RESULT.json').write_text(json.dumps({'scope':__doc__,'same_source_closure_except_morph_copy_calls':True,'no_samples_discarded':True,'runs':results},indent=2)+'\n')
