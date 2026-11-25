#!/bin/bash
set -euo pipefail
G="$(cd "$(dirname "$0")" && pwd)"; B="$(dirname "$G")"; R="$(dirname "$B")"; SRC="$G/tree"
export JIELI_TOOLCHAIN="$R/prereqs/jieli-linux-toolchains-20260730.1" AC79_SDK="$R/prereqs/sdk"
export PYTHONPATH="$SRC" DNBOX_CONTROL_FLAGS="$B/compile-flags.txt" D9_LAB_OUT="$SRC/labout"
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1 X0X_ARP_HOOKS=1 X0X_REMOTE_DNR3=1 X0X_REMOTE_TYPED_SYNTH=1
cd "$SRC"
python3 "$G/link-guarded-exact.py" --board smk37 --model elite --sdk "$AC79_SDK" > "$G/elite.log" 2>&1 & a=$!
python3 "$G/link-guarded-exact.py" --board smk37 --model pro --sdk "$AC79_SDK" > "$G/pro.log" 2>&1 & b=$!
rc=0; wait "$a" || rc=1; wait "$b" || rc=1
tail -12 "$G/elite.log"; tail -12 "$G/pro.log"; exit "$rc"
