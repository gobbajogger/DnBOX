from pathlib import Path
import json,subprocess,importlib.util,hashlib
out=Path('sound-v2-evidence/quad-activation-green').resolve();base=Path('sound-v2-evidence/replay-bundle/restored').resolve()
tool=Path('sound-v2/tests/host/run_synthone_factory.py');tool_bytes=tool.read_bytes();snapshot=out/'full-suite-factory-helper.py';snapshot.write_bytes(tool_bytes)
f=type('FactoryHelpers',(),{})();exec(compile(tool_bytes,str(snapshot),'exec'),f.__dict__)
allowed={}
for name,folder in [('TESTED-SOURCES.json','closure'),('ADAPTER-SOURCES.json','adapter-inputs')]:
 for path,h in json.loads((out/name).read_bytes()).items():allowed[out/folder/f.relative(path)]=h
allowed[snapshot]=hashlib.sha256(tool_bytes).hexdigest()
wrapper=out/'full-suite-adapter.cpp';wrapper.write_text('#include "tests/source_scheduler_test.cpp"\n#include "quad_guard.hpp"\n');allowed[wrapper]=hashlib.sha256(wrapper.read_bytes()).hexdigest()
f.verify_files(allowed)
cmds=json.loads((out/'commands.json').read_bytes());matches=[c for c in cmds if '-c'in c and c[c.index('-c')+1].endswith('/synthone_renderer_test.cpp')];assert len(matches)==1
c=list(matches[0]);oldobj=c[c.index('-o')+1];c[c.index('-c')+1]=str(wrapper);c[c.index('-o')+1]=str(out/'full-sequence-test.o');c[c.index('-MF')+1]=str(out/'full-sequence-test.d')
link=[str(out/'full-sequence-test.o')if x==oldobj else x for x in cmds[-1]];link[link.index('-o')+1]=str(out/'full-sequence-test')
with(out/'full-sequence.log').open('w')as log:
 subprocess.run(c,check=True,stdout=log,stderr=log);f.verify_dependencies(out/'full-sequence-test.d',allowed);subprocess.run(link,check=True,stdout=log,stderr=log);f.verify_files(allowed)
 for name,size,h in [('public-morphs.f32',852176,'0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5'),('compact.bin',165292,'a5cf8182b8bc9ca0a1b034a6b63f55d2cdc1087df38d6b1433da4fb56e5eccd5')]:
  b=(base/name).read_bytes();assert len(b)==size and hashlib.sha256(b).hexdigest()==h
 run=subprocess.run([str(out/'full-sequence-test'),str(base/'public-morphs.f32'),str(base/'compact.bin')],stdout=log,stderr=log)
f.verify_files(allowed);(out/'FULL-SEQUENCE-COMMANDS.json').write_text(json.dumps([c,link],indent=2)+'\n');print('full suite exit',run.returncode);raise SystemExit(run.returncode)
