#!/bin/bash
set -euo pipefail
root=$(realpath "$1"); q=$(realpath "$(dirname "$0")"); t=$(realpath "$2"); s=$(realpath "$3"); cd "$t"
tc="$root/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
"$tc/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_AUTOPAN_PROBE=1 -DX0X_S1_AUTOPAN_PORT=1 -DX0X_SJ6_NATIVE_POSITIVE_GATE=1 -DX0X_SJ6_MONO32_PROBE=1 -DX0X_S1C=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -idirafter "$tc/include" -Ifirmware/src -Ifirmware/hal -Iexperiments/original_synth -Iexperiments/wobble -Ibuild/gen -S -emit-llvm "$q/combined-layout.c" -o "$s/gate-layout.ll"
