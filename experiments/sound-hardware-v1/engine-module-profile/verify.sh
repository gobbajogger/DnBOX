#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
T=../prereqs/jieli-linux-toolchains-20260730.1/pi32v2/bin/nm
for model in elite pro; do
 D=../fw-ramtext-next/tree/joint-diagnostic-later/$model
 python engine_profile.py dual --elf "$D/never-install.elf" --link-map "$D/lab.map" --nm "$T"
 if python engine_profile.py synth-one --elf "$D/never-install.elf" --link-map "$D/lab.map" --nm "$T"; then exit 2; fi
 if python engine_profile.py superj6 --elf "$D/never-install.elf" --link-map "$D/lab.map" --nm "$T"; then exit 2; fi
done
python - <<'PY'
from engine_profile import apply_environment,plan
for p in ('dual','synth-one','superj6'):
 s={'DNBOX_ENGINE_PROFILE':p,'X0X_SYNTH_CONTROLS_INIT_ONLY':'1'}
 apply_environment(s)
 assert all(s[k]==v for k,v in plan(p).items())
 assert s['X0X_SYNTH_CONTROLS_INIT_ONLY']=='1'
 assert plan(p)['X0X_ARP_HOOKS']=='1'
for s in ({'DNBOX_ENGINE_PROFILE':'synth-one','X0X_SUPERJ6_FACTORY':'1'},
          {'DNBOX_ENGINE_PROFILE':'other'}):
 try:apply_environment(s)
 except ValueError: pass
 else:raise AssertionError(s)
s={'a':'b'}
apply_environment(s); assert s=={'a':'b'}
print("Three selections, two fail-closed conflict guards, unchanged unprofiled default")
PY
