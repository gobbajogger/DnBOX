#!/bin/bash
set -euo pipefail
root=$1;w=$2; f="$root/perf-final-proof"; b="$f/controls-init-only/source-wheel-native-scale"; l="$f/init-fdata-readonly-lab"
cd "$w/src"; mkdir -p "$w/src/gatelab"
export JIELI_TOOLCHAIN="$root/prereqs/jieli-linux-toolchains-20260730.1" PYTHONPATH="$w/src" D9_LAB_OUT="$w/src/gatelab" PYTHONDONTWRITEBYTECODE=1
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1
for model in elite pro;do
 case "$model" in pro) obj=smk37;; *) obj=smk37-$model;; esac
 mkdir -p "build/$obj"; cp -a "$b/build/$obj/gen" "build/$obj/"
 python "$l/link-sealed.py" --board smk37 --model "$model" --sdk "$root/prereqs/sdk" >"$w/gate-$model.log" 2>&1 || { tail -35 "$w/gate-$model.log";exit 1; }
 grep '^guarded' "$w/gate-$model.log"; rm -rf "build/$obj"
done
