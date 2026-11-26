#!/bin/bash
set -eu
D="$(cd "$(dirname "$0")";pwd)"; R="$(cd "$D/../..";pwd)"
export JIELI_TOOLCHAIN="$R/prereqs/jieli-linux-toolchains-20260730.1" AC79_SDK="$R/prereqs/sdk"
export PYTHONPATH="$D/tree" DNBOX_CONTROL_FLAGS="$D/compile-flags.txt" D9_LAB_OUT="$D/tree/widen-native"
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=0 X0X_SYNTHONE_PATCH=1 X0X_ARP_HOOKS=1 X0X_REMOTE_DNR3=1 X0X_REMOTE_TYPED_SYNTH=1
cd "$D/tree"
python3 "$D/compile-no-link.py" --board smk37 --model elite --sdk "$AC79_SDK" > "$D/elite.log" 2>&1 & a=$!
python3 "$D/compile-no-link.py" --board smk37 --model pro --sdk "$AC79_SDK" > "$D/pro.log" 2>&1 & b=$!
rc=0; wait "$a" || rc=1; wait "$b" || rc=1
tail -n 8 "$D/elite.log"; tail -n 8 "$D/pro.log"; exit "$rc"
