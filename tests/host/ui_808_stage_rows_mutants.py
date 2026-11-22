#!/usr/bin/env python3
"""Actual owner-bound rows reject a missing live tick or wrong owner selection."""
import os,signal,subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2]
base=root/'build/host/808-stage-row-mutants';base.mkdir(parents=True,exist_ok=True)
out=Path(tempfile.mkdtemp(prefix='run-',dir=base));(base/'latest-path.txt').write_text(str(out)+'\n')
source=(root/'firmware/src/app/ui.c').read_bytes();inc=(root/'firmware/src/app/ui_drum_voice.inc').read_text()
for name,old,new,marker in [
 ('missing-fall-time','if(decay_owner==D8S_CP+1)dv_nominal_stage_time(decay.fall*4.60517f,132-off);','/* negative: omit nominal CP fall duration */','FIXED_OWNER_STAGE_LABEL_MISSING'),
 ('missing-tick','cv_rect(230,y,3,1,SOUND_INK);','(void)y; /* negative: omit actual owner tick */','ACTUAL_OWNER_LEVEL_TICK_MISSING'),
 ('missing-preview','engine_drum808_param_curve_preview(D8_CP,3,choice,matrix_value(pg.r[0][1]),&attack)','engine_drum808_param_curve(D8_CP,3,&attack)','HELD_MA_TARGET_CURVE_MISMATCH'),
 ('wrong-owner','int selected=decay_owner==D8S_MA+1;','int selected=decay_owner==D8S_CP+1;','FIXED_OWNER_STAGE_LABEL_MISSING')]:
 if inc.count(old)!=1:raise SystemExit('missing/duplicate mutation anchor '+name)
 dest=out/name;dest.mkdir();(dest/'ui.c').write_bytes(source);(dest/'ui_drum_voice.inc').write_text(inc.replace(old,new))
 env=dict(os.environ,SMC_UI_OUT=str(dest),SMC_UI_TEST_FILE='tests/host/ui_808_stage_rows_test.c',X0X_HOST_DEFS=f'-I{root}/firmware/src/app -DX0X_HOST_UI_SOURCE="{dest}/ui.c"')
 p=subprocess.Popen(['sh','tests/host/smc_ui_test.sh'],cwd=root,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,start_new_session=True)
 try:log,_=p.communicate(timeout=120)
 except subprocess.TimeoutExpired:
  try:os.killpg(p.pid,signal.SIGKILL)
  except ProcessLookupError:pass
  log,_=p.communicate();(dest/'runtime.log').write_text(log);raise SystemExit('timeout is not a negative pass')
 (dest/'runtime.log').write_text(log)
 if p.returncode==0 or marker not in log or 'FAIL ' not in log:raise SystemExit('missing intended runtime rejection '+name+'\n'+log[-2000:])
print('PASS actual UI rejects wrong owner, missing native level tick and missing held preview')
