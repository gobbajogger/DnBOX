#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Demonstrate actual header regression fixture rejects missing R22 draws/cache keys."""
import argparse,hashlib,json,os,shutil,subprocess
from pathlib import Path

def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def change(s,a,b):
 if s.count(a)!=1:raise ValueError('Expected unique mutation anchor: '+a)
 return s.replace(a,b,1)
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();source=a.source.resolve();out=a.output.resolve();out.mkdir(parents=True,exist_ok=False);frozen=out/'source';frozen.mkdir()
 for n in ('firmware','host','tests','tools','assets','experiments'):shutil.copytree(source/n,frozen/n,symlinks=True,ignore=shutil.ignore_patterns('__pycache__'))
 inputs={str(p.relative_to(frozen)):digest(p) for p in frozen.rglob('*') if p.is_file()};(out/'inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
 ref=frozen/'tests/host/reference/ui_r22_d13_header_reference.inc';receipt=json.loads((ref.with_suffix('.json')).read_text());assert digest(ref)==receipt['referenceSHA256']
 ui=frozen/'firmware/src/app/ui.c';s=ui.read_text()
 s=change(s,'static UI_DRAW void draw_header(void)','static int fwui_header_mutation;\nstatic UI_DRAW void draw_header(void)')
 s=change(s,'dv_header_battery(219, 5);','if(fwui_header_mutation!=1)dv_header_battery(219, 5);')
 s=change(s,'snapshot.battery=(uint8_t)plat_battery();','snapshot.battery=fwui_header_mutation==2?0:(uint8_t)plat_battery();')
 s=change(s,'snapshot.sel = is_drum()?(uint8_t)ui.sel[ui.part]:0;','snapshot.sel = fwui_header_mutation==3?0:is_drum()?(uint8_t)ui.sel[ui.part]:0;')
 s=change(s,'snapshot.tempo=seq_tempo(&seq);','snapshot.tempo=fwui_header_mutation==4?0:seq_tempo(&seq);');ui.write_text(s)
 fixture=frozen/'tests/host/composed_ui_identity_test.c';s=fixture.read_text();s=change(s,'int main(void)\n{','int main(void)\n{\n    const char *mutation=getenv("FWUI_HEADER_MUTATION");fwui_header_mutation=mutation?atoi(mutation):0;');fixture.write_text(s)
 expected={str(ui.relative_to(frozen)):digest(ui),str(fixture.relative_to(frozen)):digest(fixture)}
 def verify():
  changed={p for p,h in inputs.items() if digest(frozen/p)!=h}
  if changed!=set(expected) or any(digest(frozen/p)!=h for p,h in expected.items()):raise ValueError('Mutation source drift')
 verify();env=os.environ.copy();env.update(SMC_UI_TEST_FILE='tests/host/composed_ui_identity_test.c',SMC_UI_OUT=str(out/'build'),FWUI_HEADER_MUTATION='0')
 with (out/'healthy.log').open('w')as log:r=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=frozen,env=env,stdout=log,stderr=subprocess.STDOUT)
 verify()
 if r.returncode or 'Composed UI identity:' not in (out/'healthy.log').read_text():raise ValueError('Healthy control failed')
 results=[]
 for mode,name in [(1,'missing-battery-draw'),(2,'missing-battery-cache-key'),(3,'missing-selected-voice-cache-key'),(4,'missing-tempo-cache-key')]:
  env['FWUI_HEADER_MUTATION']=str(mode)
  with (out/(name+'.log')).open('w')as log:r=subprocess.run([str(out/'build/test')],cwd=frozen,env=env,stdout=log,stderr=subprocess.STDOUT)
  verify();log=(out/(name+'.log')).read_text()
  if r.returncode!=1 or 'HEADER_MISMATCH' not in log or '!memcmp(reference_pixels,fb,240*HDR_H*2)' not in log:raise ValueError('Mutation not rejected by header identity: '+name)
  results.append(dict(mutation=name,exitCode=r.returncode,logSHA256=digest(out/(name+'.log'))))
 report=dict(schema='dnbox-r22-header-mutants/1',inputManifestSHA256=digest(out/'inputs.json'),referenceSHA256=digest(ref),instrumented=expected,healthyLogSHA256=digest(out/'healthy.log'),mutations=results,scope='One isolated actual production source with runtime-selected test mutations; healthy mode preserves source behavior, all four broken draw/cache modes must fail actual header raster identity. Not hardware/visual approval.')
 (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
if __name__=='__main__':main()
