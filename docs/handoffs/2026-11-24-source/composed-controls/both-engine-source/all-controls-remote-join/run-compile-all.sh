#!/bin/bash
set -euo pipefail
B="$(cd "$(dirname "$0")" && pwd)"
R="$(dirname "$B")"
T="$B/combined-source"
export JIELI_TOOLCHAIN="$R/prereqs/jieli-linux-toolchains-20260730.1" AC79_SDK="$R/prereqs/sdk"
export PYTHONPATH="$T" DNBOX_CONTROL_FLAGS="$B/compile-flags.txt"
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1 X0X_ARP_HOOKS=1 X0X_REMOTE_DNR3=1 X0X_REMOTE_TYPED_SYNTH=1
cd "$T"
python3 "$B/compile-all-controls.py" --board smk37 --model elite --sdk "$AC79_SDK" > "$B/elite-compile.log" 2>&1 & e=$!
python3 "$B/compile-all-controls.py" --board smk37 --model pro --sdk "$AC79_SDK" > "$B/pro-compile.log" 2>&1 & p=$!
rc=0; wait "$e" || rc=1; wait "$p" || rc=1
tail -15 "$B/elite-compile.log"; tail -15 "$B/pro-compile.log"
exit "$rc"
