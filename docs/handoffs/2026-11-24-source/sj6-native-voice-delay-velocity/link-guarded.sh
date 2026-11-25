#!/bin/bash
# Prepared isolated source directory only. Links into a guarded lab, never packages.
set -euo pipefail
root=$(realpath "$1"); t=$(realpath "$2"); w=$(realpath "$3")
f="$root/perf-final-proof"; l="$f/init-fdata-readonly-lab"
sed '/^b.CFLAGS += /a b.CFLAGS += ["-DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1","-DX0X_S1_MOD_RATE_PROBE=1","-DX0X_S1_BEND_RANGE_PROBE=1","-DX0X_S1_AUTOPAN_PROBE=1","-DX0X_S1_AUTOPAN_PORT=1","-DX0X_SJ6_MONO32_PROBE=1"]' "$l/link-sealed.py" > "$w/port-link.py"
if [[ "${POSITIVE_GATE:-0}" == 1 ]]; then sed -i '/^b.CFLAGS += /a b.CFLAGS.append("-DX0X_SJ6_NATIVE_POSITIVE_GATE=1")' "$w/port-link.py"; fi
sed -i '/^b.CFLAGS += /a b.CFLAGS.extend(["-DX0X_S1_FEEDBACK_PROBE=1","-DX0X_S1_DELAY_TRACK_TEST=1","-DX0X_SJ6_FILTER_VEL_PROBE=1","-DX0X_SJ6_VELOCITY_PORT=1","-DX0X_SJ6_NATIVE_RANDOM2=1"])' "$w/port-link.py"
cd "$t"
export JIELI_TOOLCHAIN="$root/prereqs/jieli-linux-toolchains-20260730.1" PYTHONPATH="$t" D9_LAB_OUT="$t/labout" PYTHONDONTWRITEBYTECODE=1
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1
for m in elite pro; do
  case "$m" in pro) obj=smk37;; *) obj=smk37-$m;; esac
  mkdir -p "build/$obj"
  cp -a "$f/controls-init-only/source-wheel-native-scale/build/$obj/gen" "build/$obj/"
  python "$w/port-link.py" --board smk37 --model "$m" --sdk "$root/prereqs/sdk" >"$w/$m-link.log" 2>&1 || { tail -35 "$w/$m-link.log"; exit 1; }
  grep '^guarded' "$w/$m-link.log"
  rm -rf -- "build/$obj"
  if [[ "${KEEP_LINK:-0}" != 1 ]]; then rm -rf -- "$t/labout/$m"; fi
done
