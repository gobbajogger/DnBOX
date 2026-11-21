#!/usr/bin/env python3
"""Four actual SMC caption scenarios plus independently reintroduced source defects."""
from pathlib import Path
import json,os,subprocess
ROOT=Path(__file__).resolve().parents[2]
def main():
 out=Path(os.environ.get('SMC_UI_OUT',ROOT/'build/host/smc-caption-geometry')).resolve();out.mkdir(parents=True,exist_ok=True)
 env=os.environ.copy();env.update(SMC_UI_TEST_FILE='tests/host/smc_caption_geometry_test.c',SMC_UI_OUT=str(out/'positive'))
 p=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=ROOT,env=env);assert p.returncode==0,'SMC caption positive failed'
 source=(ROOT/'firmware/src/app/smc_mixer.c').read_text();mutants={
 'pattern-sound':('            if (k.kind == R_BPATSND) label = "SOUND";\n',''),
 'button-edge':('text_c(x + 18, text_y, font, caption, c);','text_c(x + 19, text_y, font, caption, c);')}
 receipts={}
 for name,(before,after) in mutants.items():
  assert source.count(before)==1,'nonunique mutation '+name
  path=out/(name+'.c');path.write_text(source.replace(before,after,1));where=out/name
  env.update(SMC_UI_SOURCE_FILE=str(path),SMC_UI_OUT=str(where))
  p=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=ROOT,env=env,capture_output=True,text=True);log=p.stdout+p.stderr;(out/(name+'.log')).write_text(log)
  assert p.returncode!=0 and (where/'test').is_file(),'negative did not compile and run '+name
  assert 'SMC_CAPTION_GEOMETRY' in log and 'FAIL tests/host/smc_caption_geometry_test.c:' in log,'wrong failure '+name
  receipts[name]={'exit':p.returncode,'compiled':True,'intended_runtime_marker':'SMC_CAPTION_GEOMETRY'}
 (out/'negative-receipts.json').write_text(json.dumps(receipts,indent=2)+'\n');print('PASS SMC caption geometry + both source-defect runtime negatives')
if __name__=='__main__':main()
