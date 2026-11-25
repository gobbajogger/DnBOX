#!/bin/bash
set -euo pipefail
d=$(realpath "$(dirname "$0")"); root=$(realpath "$1"); src=$(realpath "$2"); carrier=$(realpath "$3"); w=$(realpath "$4"); m="${5:-elite}"
[[ "$m" == elite || "$m" == pro ]] || exit 2
l="$root/perf-final-proof/init-fdata-readonly-lab"; base="$root/perf-final-proof/controls-init-only/source-wheel-native-scale"
(cd "$src"; sha256sum --quiet -c "$root/perf-final-proof/wheels-page24-classic-random2-join/POSTIMAGE.sha256")
mkdir -p "$w/src/build" "$w/src/labout"
cd "$w/src"
for x in firmware tools experiments; do cp -a "$(realpath "$src/$x")" .; done
for x in assets ble; do ln -s "$(realpath "$src/$x")"; done
grep -v "typed_controls_trial.h" "$carrier/PREIMAGE.sha256" | sha256sum --quiet -c -
test ! -e firmware/src/app/typed_controls_trial.h
patch --fuzz=0 -s -p1 < "$carrier/native-trial-carrier.patch"
sha256sum --quiet -c "$carrier/POSTIMAGE.sha256"
patch --fuzz=0 -s -p1 < "$d/autopan-port.patch"
patch --fuzz=0 -s -p1 < "$root/perf-final-proof/init-only-legacy-root-prune/prune.patch"
patch --fuzz=0 -s -p1 < "$l/boot.diagnostic.diff"
cp "$l/fdata_readonly_lab.h" firmware/src/app/
# Preserve the previously native-qualified no-libm default wheel scaling.
python3 - "$root/perf-final-proof/s1-mod-native-scale/native-scale.patch" <<'PY'
import sys
from pathlib import Path
p=Path("firmware/src/app/s1c_part.c"); s=p.read_text()
old="    const float cutoff = 360.f * powf(7600.f / 120.f, (float)(127u-cc1) / 127.f);"
assert s.count(old)==1
lines=Path(sys.argv[1]).read_text().splitlines()
i=next(i for i,l in enumerate(lines) if l=="-"+old)
out=[]
for l in lines[i+1:]:
 if not l.startswith("+"): break
 out.append(l[1:])
assert len(out)>5 and "cutoff *=" in out[-1]
p.write_text(s.replace(old,"\n".join(out)))
PY
if [[ ${ONLY_CAPTURE_PREIMAGE:-0} == 1 ]]; then exit 0; fi
if [[ -s "$d/pilot-extra.patch" ]]; then patch -s --fuzz=0 -p1 < "$d/pilot-extra.patch"; fi
sed '/^b.CFLAGS += /a b.CFLAGS += ["-DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1","-DX0X_S1_MOD_RATE_PROBE=1","-DX0X_S1_BEND_RANGE_PROBE=1","-DX0X_S1_AUTOPAN_PROBE=1","-DX0X_S1_AUTOPAN_PORT=1"]' "$l/link-sealed.py" > "$w/port-link.py"
export JIELI_TOOLCHAIN="$root/prereqs/jieli-linux-toolchains-20260730.1" PYTHONPATH="$w/src" D9_LAB_OUT="$w/src/labout" PYTHONDONTWRITEBYTECODE=1
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1
case "$m" in pro) obj=smk37;; *) obj=smk37-$m;; esac
mkdir -p "build/$obj"
cp -a "$base/build/$obj/gen" "build/$obj/"
python "$w/port-link.py" --board smk37 --model "$m" --sdk "$root/prereqs/sdk"
