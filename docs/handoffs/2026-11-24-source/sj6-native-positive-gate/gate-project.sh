#!/bin/bash
set -euo pipefail
root=$1;w=$2
cd "$w/src"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
. tests/host/original_build.sh
cc -O1 -pipe -std=c11 -DX0X_HOST -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 $ORIGINAL_FLAGS -Itests/host -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen -I"$PWD" tests/host/gate-project.c $ORIGINAL_SOURCES -lm -o "$w/export-gate"
"$w/export-gate" "$w/gate-project.bin"
tc="$root/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
"$tc/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -DX0X_S1C=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -idirafter "$tc/include" -Ifirmware/src -Ifirmware/hal -Iexperiments/original_synth -Iexperiments/wobble -Ibuild/gen -S -emit-llvm "$w/gate-layout.c" -o "$w/gate-layout.ll"
grep -E '^@[a-zA-Z]+ = constant' "$w/gate-layout.ll"
