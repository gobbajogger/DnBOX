#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import os, subprocess, tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2]
out=root/'build/host/glyph-polish';out.mkdir(parents=True,exist_ok=True)
def run(name, source=None):
 env=dict(os.environ, SMC_UI_TEST_FILE='tests/host/ui_glyph_polish_test.c', SMC_UI_OUT=str(out/name))
 if source: env['X0X_HOST_DEFS']=env.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(source)+'"'
 p=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=root,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120)
 (out/(name+'.log')).write_text(p.stdout)
 return p
p=run('positive')
if p.returncode or 'GLYPH POLISH PASS' not in p.stdout: raise SystemExit(p.stdout[-4000:])
s=(root/'firmware/src/app/ui.c').read_text()
a='text_c(31+e.slot[k]*59,y+8-off,&FONT_T,label,edited==k?hi:SOUND_GRAY);'
mutations=[('missing-caption',a,'(void)label;'),('wrong-slot','UI_FIRST_KNOB+e.slot[k]','UI_FIRST_KNOB+((e.slot[k]+1)%4)'),('wrong-stage','k==0?"A ":k==1?"D ":k==2?"S ":"R "','k==0?"D ":k==1?"A ":k==2?"S ":"R "'),('wrong-percentage','(matrix_value(pg.r[p][e.slot[k]])*100+63)/127','0')]
for name,a,b in mutations:
 if s.count(a)!=1: raise SystemExit('missing/duplicate anchor '+name)
 with tempfile.TemporaryDirectory(prefix='glyph-caption-') as d:
  f=Path(d)/'ui.c';f.write_text(s.replace(a,b));p=run(name,f)
  if p.returncode==0 or 'PHYSICAL_ADSR_CAPTION_MISMATCH' not in p.stdout or 'FAIL ' not in p.stdout: raise SystemExit('intended runtime mutant not rejected '+name+' '+p.stdout[-4000:])
print('PASS full glyph polish and four physical ADSR caption runtime negatives')
