#!/usr/bin/env python3
"""Exploratory ABBA same-host measurement, not hard realtime qualification."""
import pathlib,hashlib,json,subprocess,importlib.util,os
root=pathlib.Path(__file__).resolve().parent
roots={'A':root/'callback-bitcopy-pilot','B':root/'callback-distinct-pilot'}
original_affinity=sorted(os.sched_getaffinity(0));chosen=max(original_affinity)
os.sched_setaffinity(0,{chosen});assert os.sched_getaffinity(0)=={chosen}
output=root/'callback-distinct-affinity-diagnostic';output.mkdir(exist_ok=False)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
inputs={k:json.loads((p/'CALLBACK-INPUTS.json').read_bytes()) for k,p in roots.items()}
assert inputs['A'].keys()==inputs['B'].keys()
assert {k for k in inputs['A'] if inputs['A'][k]!=inputs['B'][k]}=={'closure/scheduler/compact-reader/compact.c'}
exe={k:sha(p/'callback-bench') for k,p in roots.items()}
assert exe['A']==json.loads((roots['A']/'BUILD-IDENTITY.json').read_bytes())['executable_sha256']
assert exe['B']==json.loads((roots['B']/'BUILD-IDENTITY.json').read_bytes())['executable_sha256']
def verify():
 for k,p in roots.items():
  assert sha(p/'callback-bench')==exe[k]
  for f,h in inputs[k].items():assert sha(p/f)==h,(k,f)
verify()
runner=(roots['B']/'callback-runner.py').read_bytes();ns={'__name__':'paired_callback_helpers'};exec(compile(runner,'<verified callback runner>','exec'),ns)
reference_path=root/'callback-bitcopy-abba/1-B/programs.jsonl'
reference_bytes=reference_path.read_bytes();reference_hash=hashlib.sha256(reference_bytes).hexdigest()
reference=[json.loads(line) for line in reference_bytes.splitlines()]
expected=[(x['preset'],x['frames'],x['diagnostic_fnv64']) for x in reference];assert len(expected)==8
results=[]
for index,k in enumerate('ABBA'):
 verify();p=roots[k];out=output/f'{index}-{k}';out.mkdir()
 assert os.sched_getaffinity(0)=={chosen}
 cmd=[str(p/'callback-bench'),str(p/'factory-inputs/public-morphs.f32'),str(p/'factory-inputs/compact.bin'),str(p/'commands/preset-commands.bin'),str(out/'trace.csv'),str(out/'programs.jsonl'),'8','0']
 r=subprocess.run(cmd,capture_output=True,text=True);(out/'run.log').write_text(r.stdout+r.stderr);verify()
 s=ns['summarize'](out/'trace.csv',8);progs=[json.loads(x) for x in (out/'programs.jsonl').read_text().splitlines()];assert len(progs)==8
 assert [(x['preset'],x['frames'],x['diagnostic_fnv64']) for x in progs]==expected
 assert reference_path.read_bytes()==reference_bytes
 finite=sum(x['nonfinite_words'] for x in progs);assert r.returncode==int(bool(finite or s['wall_misses'] or s['cpu_misses']))
 result={'order':index,'variant':k,'executable_sha256':exe[k],'benchmark_tracer_pid':ns['parse_benchmark_tracer'](r.stderr),'summary':s,'nonfinite_words':finite,'files':{x.name:sha(x) for x in out.iterdir()}}
 results.append(result);(out/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result),flush=True)
(output/'RESULT.json').write_text(json.dumps({'scope':'Fixed-process-affinity diagnostic only; no scheduler isolation or realtime qualification','original_affinity':original_affinity,'configured_affinity':[chosen],'same_source_closure_except_duplicate_coordinate_reads':True,'no_samples_discarded':True,'reference_programs_sha256':reference_hash,'diagnostic_program_digests_preserved':True,'runs':results},indent=2)+'\n')

os.sched_setaffinity(0,set(original_affinity))
