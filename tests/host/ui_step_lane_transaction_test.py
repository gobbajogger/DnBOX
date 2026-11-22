#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Observe real CQ transaction boundaries and reject a current-source stale-read mutant."""
import json
import os
from pathlib import Path
import re
import subprocess

ROOT=Path(__file__).resolve().parents[2]
OUT=Path(os.environ.get('SMC_UI_OUT',str(ROOT/'build/host/step-lane-transaction'))).resolve()
OUT.mkdir(parents=True,exist_ok=True)
ui=(ROOT/'firmware/src/app/ui.c').read_text()
inc=(ROOT/'firmware/src/app/ui_step_lane.inc').read_text()

def instrument(text):
    return text.replace('plat_cq_lock()', 'regression_cq_lock()').replace('plat_cq_unlock(', 'regression_cq_unlock(')

def prepare(name,damaged):
    target=OUT/name;target.mkdir(exist_ok=True)
    body=inc
    if damaged:
        guard='    if(part==PART_BRK)state=plat_cq_lock();\n'
        anchor='    next=value+e;'
        if body.count(guard)!=1 or body.count(anchor)!=1:raise ValueError('missing stale-read mutation anchor')
        body=body.replace(guard,'',1).replace(anchor,guard+anchor,1)
    anchors=[('    pref_t ref=NONE;','    regression_prepare_scope(part);\n    pref_t ref=NONE;'),
             ('    int value=c->fallback;*conditional=0;','    regression_read_scope(c->part);\n    int value=c->fallback;*conditional=0;'),
             ('        proj.arr.lane[k].val[step]=(uint8_t)next;','        regression_store_scope(part);\n        proj.arr.lane[k].val[step]=(uint8_t)next;')]
    for old,new in anchors:
        if body.count(old)!=1:raise ValueError('missing observer anchor '+old)
        body=body.replace(old,new,1)
    (target/'ui.c').write_text(instrument(ui));(target/'ui_step_lane.inc').write_text(instrument(body))
    return target

results=[]
for name,damaged in [('positive',False),('stale-read',True)]:
    target=prepare(name,damaged)
    env=dict(os.environ,SMC_UI_OUT=str(target/'host'),SMC_UI_TEST_FILE='tests/host/ui_step_lane_transaction_test.c')
    env['X0X_HOST_DEFS']=env.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(target/'ui.c')+'"'
    run=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    (target/'run.log').write_text(run.stdout)
    if not damaged:
        if run.returncode or 'PASS BREAK lane transaction:' not in run.stdout:raise SystemExit(run.stdout[-5000:])
    elif run.returncode!=1 or 'FAIL BREAK_TRANSACTION_STALE_READ' not in run.stdout:
        raise SystemExit('stale-read mutant did not fail at the intended runtime check\n'+run.stdout[-5000:])
    results.append({'case':name,'exit':run.returncode,'log':str(target/'run.log')})
(OUT/'result.json').write_text(json.dumps({'status':'PASS','cases':results,'scope':'Real input/editor code with substituted host CQ guard observers; no device/IRQ timing qualification'},indent=2)+'\n')
print('PASS BREAK transaction scopes and current-source stale-read runtime negative')
