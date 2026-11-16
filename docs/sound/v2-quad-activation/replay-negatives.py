import pathlib,json,hashlib,subprocess
out=pathlib.Path('sound-v2-evidence/quad-activation-green').resolve();base=pathlib.Path('sound-v2-evidence/replay-bundle/restored').resolve();allowed={}
for f,root in [('TESTED-SOURCES.json','closure'),('ADAPTER-SOURCES.json','adapter-inputs')]:
 for n,h in json.loads((out/f).read_text()).items():allowed[out/root/n]=h
exe=out/'adapter-test';allowed[exe]=hashlib.sha256(exe.read_bytes()).hexdigest()
for n,h in [('public-morphs.f32','0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5'),('compact.bin','a5cf8182b8bc9ca0a1b034a6b63f55d2cdc1087df38d6b1433da4fb56e5eccd5')]:allowed[base/n]=h
def verify():
 for p,h in allowed.items():assert hashlib.sha256(p.read_bytes()).hexdigest()==h,p
results=[]
for mode,expected in [('quad-wrong-word','different==0'),('quad-oracle-window','!compact_oracle_window')]:
 verify();r=subprocess.run([str(exe),str(base/'public-morphs.f32'),str(base/'compact.bin'),str(out/(mode+'.pcm')),mode],capture_output=True,text=True);verify();(out/(mode+'.log')).write_text(r.stdout+r.stderr);assert r.returncode==1 and expected in r.stderr,(mode,r.returncode,r.stderr);results.append({'mode':mode,'exit':r.returncode,'expected_failure':expected,'log_sha256':hashlib.sha256((r.stdout+r.stderr).encode()).hexdigest()})
(out/'QUAD-NEGATIVES.json').write_text(json.dumps({'scope':'Real quad-specific wrong-word/oracle-window failures, no mocked reader','results':results,'executable_sha256':allowed[exe]},indent=2)+'\n');print(results)
