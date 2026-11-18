#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Real contextual value agrees with native law; old raw-percent bug rejected."""
import os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/host/ui-fx-feedback-context'
OUT.mkdir(parents=True,exist_ok=True)
def run(name,source=None):
 env=os.environ.copy();env.update(SMC_UI_TEST_FILE='tests/host/ui_fx_feedback_context_test.c',SMC_UI_OUT=str(OUT/name))
 if source:env['X0X_HOST_DEFS']=env.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(source)+'"'
 p=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120)
 (OUT/(name+'.log')).write_text(p.stdout);return p
p=run('normal')
if p.returncode:print(p.stdout[-5000:]);raise SystemExit(p.returncode)
if 'PASS FX configured feedback' not in p.stdout:raise SystemExit('missing positive runtime completion')
with tempfile.TemporaryDirectory(prefix='dnbox-fx-context-negative-') as tmp:
 source=(ROOT/'firmware/src/app/ui.c').read_text()
 good='    pref_value(PR(R_ENG,T_FX,0,FX_DL_FDBK),a,u);\n    put_s(put_s(t,a),u);'
 old='    put_s(put_i(t,proj.sound.v[T_FX][0][FX_DL_FDBK]*100/127),"%");'
 if source.count(good)!=1:raise SystemExit('missing/duplicate physical formatter anchor')
 path=Path(tmp)/'ui.c';path.write_text(source.replace(good,old));p=run('old-raw-percent',path)
 if p.returncode==0 or 'feedback-context-mismatch' not in p.stdout:
  print(p.stdout[-5000:]);raise SystemExit('old raw-percent defect did not fail actual runtime')
print('PASS FX context native law and old-raw-percent runtime negative')
