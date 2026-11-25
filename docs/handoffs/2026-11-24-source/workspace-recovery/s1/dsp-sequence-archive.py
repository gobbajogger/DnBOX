from pathlib import Path
import re, subprocess, hashlib
q=Path('recovery/source/docs/docs/handoffs/2026-11-24-source')
r=Path('recovery/s1/dsp-sequence-archive').resolve();out=Path('recovery/s1/exact-files')
files=['s1c.c','s1c.h']
n='firmware/src/dsp/s1c/'
stages=[
'perf-final-proof/sj6-knob-persistence/engine-first-input.patch',
'ui-1340-reconciled/synthone-oscillator-editor.u1xb7D/synthone-compact-editor.patch',
'ui-1340-reconciled/synthone-sync-join.s979TF/synthone-pwm2-sync-owner.patch',
'perf-final-proof/full-control-s1-lfo-rebase.qADro3/s1-lfo-full-control.patch',
'controls-init-only/implemented-controls-aggregate.patch',
's1-mod-rate-probe/cc1-rate-probe.patch',
'wheels-a4-rv-typed-join/wheels-a4-rv.patch',
'wheels-activegate-crush-typed-join/activegate-crush.patch',
'wheels-activegate-crush-delay-autopan-typed-join/autopan-join.patch',
'wheels-autopan-bpm-feedback-typed-join/feedback-join.patch',
's1-autopan-native-controls/autopan-port.patch',
'sj6-native-voice-delay-velocity/delta.patch',
]
for x in stages:
 s=(q/x).read_text()
 for f in files:
  match=re.search(r'^--- a/'+re.escape(n+f)+r'\n.*?(?=^diff --git |^--- |\Z)',s,re.M|re.S)
  if not match:continue
  path=r/n/f;p=path.read_bytes()
  a=subprocess.run(['patch','--batch','--forward','--fuzz=0','--no-backup-if-mismatch','-p1','-r','-'],input=match[0].encode(),stdout=subprocess.PIPE,cwd=r)
  if a.returncode:
   path.write_bytes(p)
   print('MISS',f,Path(x).name)
  else: print('APPLY',f,Path(x).name,hashlib.sha256(path.read_bytes()).hexdigest()[:12])
for line in (q/'sj6-native-voice-delay-velocity/FIRMWARE.sha256').read_text().splitlines():
 h,p=line.split()
 if p in [n+x for x in files]:
  b=(r/p).read_bytes();a=hashlib.sha256(b).hexdigest()
  if a==h:
   f=out/p;f.parent.mkdir(parents=True,exist_ok=True);f.write_bytes(b);print('EXACT',p)
  else:print('NOTEXACT',p,a,h)
