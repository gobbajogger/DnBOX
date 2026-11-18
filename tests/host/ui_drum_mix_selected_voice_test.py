#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Independent recovered R22 binding oracle and actual drum MIX gestures."""
import os,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/host/mix-selected-edges'
OUT.mkdir(parents=True,exist_ok=True)
def run(name,source=None):
    env=dict(os.environ,SMC_UI_TEST_FILE='tests/host/ui_drum_mix_selected_voice_test.c',SMC_UI_OUT=str(OUT/name))
    if source:env['X0X_HOST_DEFS']=env.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(source)+'"'
    p=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=150)
    (OUT/(name+'.log')).write_text(p.stdout)
    return p
p=run('positive')
if p.returncode or 'PASS all22 real selected-voice MIX' not in p.stdout:raise SystemExit(p.stdout[-5000:])
s=(ROOT/'firmware/src/app/ui.c').read_text()
mutants=[('selection','ui.sel[km] = (uint8_t)bl;','ui.sel[km] = 0;','ui.sel[kit]==voice'),
 ('binding','int t = p ? T_808 : T_909, v = selection[p];','int t = p ? T_808 : T_909, v = 0;','same_ref(r,expected_mix_refs')]
for name,old,new,marker in mutants:
    if s.count(old)!=1:raise SystemExit('missing/duplicate MIX mutation anchor: '+name)
    with tempfile.TemporaryDirectory(prefix='mix-owner-negative-') as d:
        f=Path(d)/'ui.c';f.write_text(s.replace(old,new));p=run(name,f)
        if p.returncode==0 or 'FAIL ' not in p.stdout or marker not in p.stdout:raise SystemExit('MIX mutation did not fail at runtime: '+p.stdout[-5000:])
print('PASS all22 MIX physical binding/list/edge contracts and wrong-owner runtime negatives')
