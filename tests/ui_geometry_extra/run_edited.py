#!/usr/bin/env python3
"""Audit all20 actual persisted lane-edit UI frames; source/fixture unchanged."""
from pathlib import Path
import argparse,shutil,os,subprocess,json
from verify import verify

def run(source,out):
 out.mkdir(parents=True,exist_ok=False);work=out/'source';shutil.copytree(source,work,symlinks=True,ignore=shutil.ignore_patterns('.git','build','__pycache__'))
 text=(work/'tests/ui_geometry/capture.c').read_text();assert text.count('int main(void)')==1
 text=text.replace('int main(void)','int f2_inventory_unused_main(void)')
 fixture=(work/'tests/host/ui_step_lane_edit_test.c').read_text().split('#include "smc_ui_test.c"',1)[1]
 fixture=fixture.replace('int main(void)\n{','int main(void)\n{\n f2_extra_enabled=1;f2_reset();')
 (work/'tests/ui_geometry/capture-edited.c').write_text(text+'\n'+fixture)
 env={k:v for k,v in os.environ.items() if 'MUTATE' not in k};env.update(SMC_UI_TEST_FILE='tests/ui_geometry/capture-edited.c',SMC_UI_OUT=str(out/'host'))
 with (out/'positive.log').open('w') as log:p=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=work,env=env,stdout=log,stderr=subprocess.STDOUT)
 assert p.returncode==0,'edited-frame runtime failed';positive=verify(out/'positive.log');assert positive['frames']==20,'incomplete edited-family frames'
 negative_dir=out/'negative-shots';negative_dir.mkdir();env.update(F2_EXTRA_MUTATE='1',SMC_UI_SHOTS=str(negative_dir))
 with (out/'negative.log').open('w') as log:n=subprocess.run([str(out/'host/test')],cwd=work,env=env,stdout=log,stderr=subprocess.STDOUT)
 try:verify(out/'negative.log')
 except ValueError as e:negative={'verifier_exit':1,'reason':str(e),'renderer_exit':n.returncode}
 else:raise AssertionError('damaged frame was accepted')
 assert 'F2_EXTRA_FINDING' in (out/'negative.log').read_text(),'wrong negative'
 report={'status':'PASS','positive':positive,'negative':negative,'scope':'20complete UI_FRAME fixtures after physical lane edits and host project persistence; opaque text masks only'}
 (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--instrumented-extra-source',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();run(a.instrumented_extra_source.resolve(),a.output.resolve())
