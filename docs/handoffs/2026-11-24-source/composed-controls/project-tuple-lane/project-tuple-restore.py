"""Reconcile project admission source; does not run a build or test."""
import re,sys
from hashlib import sha256
from pathlib import Path as P
from difflib import unified_diff
n='firmware/src/app/project.c'
h=P('restored-handoff/docs/handoffs/2026-11-24-source')
pre=(P('all-controls-source-join/tree')/n).read_text(); s=pre
pp=['sj6-id137-joined-carriage/combined-host.patch','id137-generation-bind/tuple-binding.patch','s1-mod-sidecar/s1-mod-sidecar.patch','s1-mod-smk-ui/s1-mod-smk-ui.patch','typed-controls-native-admission/native-trial-carrier.patch','s1-autopan-native-controls/pilot-extra.patch','perf-final-proof/sj6-knob-persistence/superj6-oscillator-knobs.patch','perf-final-proof/sj6-hz-panel/superj6-hz-source.patch']
for f in pp:
    p=(h/f).read_text().split('+++ b/'+n+'\n')[1].split('\n--- ')[0]
    p=p.replace('(s->bt & 0xC0u)', '(s->bt & (0xC0u & ~PROJ_DEVICE_PREFS))')
    for header,b in re.findall(r'(^@@ .*?\n)(.*?)(?=^@@ |\Z)',p,re.M|re.S):
        ll=b.splitlines()
        old='\n'.join(l[1:] for l in ll if l.startswith((' ','-')))
        new='\n'.join(l[1:] for l in ll if l.startswith((' ','+')))
        if s.count(old)==1: s=s.replace(old,new,1)
        elif s.count(new)!=1: raise ValueError((f,header))
print(sha256(pre.encode()).hexdigest(),sha256(s.encode()).hexdigest(),file=sys.stderr)
print(''.join(unified_diff(pre.splitlines(keepends=True),s.splitlines(keepends=True),'a/'+n,'b/'+n)),end='')
