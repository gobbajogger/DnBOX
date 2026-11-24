#!/usr/bin/env python3
"""REVERB bank = Decay, Tone, Rev Low Cut, Rev Return (Guidelines §3). Repeat-safe M2 positive and compiled current-source runtime negatives."""
import json,os,subprocess,hashlib,signal
from pathlib import Path
root=Path(__file__).resolve().parents[2]
out=Path(os.environ.get('M2_OUT',root/'build/host/reverb-return')).resolve();out.mkdir(parents=True,exist_ok=True)
(out/'receipt.json').unlink(missing_ok=True)
source=(root/'firmware/src/app/ui.c').read_text()
front='PR(R_ENG, T_FX, 0, FX_RV_DECAY), PR(R_ENG, T_FX, 0, FX_RV_TONE), PR(R_ENG, T_FX, 0, FX_RV_HPF), PR(R_ENG, T_FX, 0, FX_RV_LEVEL)'
guard='if (ui.view != V_FX || p || k < 2)'
assert source.count(front)==1 and source.count(guard)==1,'candidate anchors missing/duplicated'
records=[]
def run(name,text=None,expected=None):
 d=out/name;d.mkdir(parents=True,exist_ok=True)
 binary=d/'test';binary.unlink(missing_ok=True)
 env=dict(os.environ,SMC_UI_TEST_FILE='tests/host/ui_reverb_return_test.c',SMC_UI_OUT=str(d))
 if text is not None:
  f=d/'ui-mutant.c';f.write_text(text)
  env['X0X_HOST_DEFS']=env.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(f)+'"'
 command=['sh','tests/host/smc_ui_test.sh']
 process=subprocess.Popen(command,cwd=root,env=env,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,start_new_session=True)
 try:
  output,_=process.communicate(timeout=180)
 except subprocess.TimeoutExpired:
  try:os.killpg(process.pid,signal.SIGKILL)
  except ProcessLookupError:pass
  output,_=process.communicate()
  (out/(name+'.log')).write_text(output)
  (out/(name+'.exit')).write_text(str(process.returncode)+'\n')
  raise SystemExit(f'{name}: timed out; owned process group {process.pid} killed and reaped')
 p=subprocess.CompletedProcess(command,process.returncode,output)
 (out/(name+'.log')).write_text(p.stdout)
 (out/(name+'.exit')).write_text(str(p.returncode)+'\n')
 compiled=binary.exists()
 record={'name':name,'compiled':compiled,'exit':p.returncode,'expected_runtime_marker':expected,'marker_observed':expected in p.stdout if expected else 'M2_RESULT' in p.stdout,'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest() if compiled else None}
 records.append(record);(out/'receipt.json').write_text(json.dumps(records,indent=2))
 if not compiled:raise SystemExit(name+': compiler failure cannot qualify a negative\n'+p.stdout[-2500:])
 if expected:
  if p.returncode==0 or expected not in p.stdout:raise SystemExit(name+': wrong/missing runtime rejection\n'+p.stdout[-2500:])
 elif p.returncode or 'failures=0' not in p.stdout:raise SystemExit(name+': positive failed\n'+p.stdout[-2500:])
run('positive')
run('wrong-owner',source.replace(front,front.replace('FX_RV_LEVEL','SWAP_RETURN').replace('FX_RV_HPF','FX_RV_LEVEL').replace('SWAP_RETURN','FX_RV_HPF')),'M2_FAIL FRONT_BANK_EXACT_OWNER')
run('missing-front-slots',source.replace(front,'PR(R_ENG, T_FX, 0, FX_RV_DECAY), PR(R_ENG, T_FX, 0, FX_RV_TONE), NONE, NONE'),'M2_FAIL K7_MISSING_REV_LOW_CUT')
# The fixture CHECK fails on exact identity at the first moved list row.
run('removed-order-guard',source.replace(guard,'if (1)'),'ref_eq(list_rows[i],refs[i])')
print('PASS M2 real framebuffer/physical owners and three compiled current-source runtime negatives')
