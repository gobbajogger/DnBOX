#!/bin/bash
set -eu
D="$(cd "$(dirname "$0")";pwd)"; R="$(cd "$D/..";pwd)"
export JIELI_TOOLCHAIN="$R/prereqs/jieli-linux-toolchains-20260730.1" AC79_SDK="$R/prereqs/sdk"
export PYTHONPATH="$D/tree" DNBOX_CONTROL_FLAGS="$D/compose-flags.txt" D9_LAB_OUT="$D/tree/joint-diagnostic-keycache"
export X0X_SUPERJ6_FACTORY=0 X0X_SUPERJ6_BUILTIN_INIT=0 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1 X0X_ARP_HOOKS=1 X0X_REMOTE_DNR3=1 X0X_REMOTE_TYPED_SYNTH=0
export D9_LAB_XDATA_TAIL=1 D9_LAB_SLOT3_METADATA=1 DNBOX_TANH_SPLIT_LAB=1 DNBOX_TANH_REFILL_LAB=1 D9_LAB_S1_FDATA=1 DNBOX_CHICAGO_MOD_PALETTE=1 DNBOX_MIAMI_STOCK_CAP_COMPACT=1 D9_LAB_SPARE_TABLES=1 DNBOX_MIAMI_DECIMATOR_COMPACT=1 DNBOX_CHICAGO_DRIVE_COMPACT=1 DNBOX_CHICAGO_MOD_SPEED_COMPACT=1
cd "$D/tree"
python3 "$D/link-sealed-s1.py" --board smk37 --model elite --sdk "$AC79_SDK" > "$D/elite-keycache.log" 2>&1 & a=$!
python3 "$D/link-sealed-s1.py" --board smk37 --model pro --sdk "$AC79_SDK" > "$D/pro-keycache.log" 2>&1 & b=$!
rc=0; wait "$a" || rc=1; wait "$b" || rc=1
tail -n 8 "$D/elite-keycache.log"; tail -n 8 "$D/pro-keycache.log"; exit "$rc"
