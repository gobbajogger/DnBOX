#!/bin/bash
set -eu
D=$(cd "$(dirname "$0")" && pwd)
R=$(dirname "$D")
P="$R/rehydrated-reverb/delay-center-exact-wet"
B="$D/target"
mkdir -p "$B"
for dir in firmware tools assets build experiments ble reference; do
 cp --reflink=auto -a "$P/tree/$dir" "$B/"
done
cd "$B"
cp "$D"/{compact.h,leaf.c,pin-bench.h} firmware/src/app/
python3 - <<'PY'
from pathlib import Path
p=Path('firmware/src/app/main_fm1.c')
s=p.read_text()
n=s.index('struct x0x_dbg {')
s=s[:n]+'#define HP_PROBE_TICKS() fm1_ticks()\n#include "pin-bench.h"\n#undef HP_PROBE_TICKS\n\n'+s[n:]
a='    plat_perf_init();'
assert s.count(a)==1
p.write_text(s.replace(a,a+'\n    sj6_hp_probe(); /* disposable emulator only */'))
PY
export JIELI_TOOLCHAIN="$R/prereqs/jieli-linux-toolchains-20260730.1" AC79_SDK="$R/prereqs/sdk" PYTHONPATH="$B"
export DNBOX_CONTROL_FLAGS="$R/all-controls-remote-join/compile-flags.txt" D9_LAB_OUT="$B/out"
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1 X0X_ARP_HOOKS=1 X0X_REMOTE_DNR3=1 X0X_REMOTE_TYPED_SYNTH=1
export D9_LAB_XDATA_TAIL=1 D9_LAB_SLOT3_METADATA=1 DNBOX_TANH_SPLIT_LAB=1 DNBOX_TANH_REFILL_LAB=1 D9_LAB_S1_FDATA=1 DNBOX_CHICAGO_MOD_PALETTE=1 DNBOX_MIAMI_STOCK_CAP_COMPACT=1 D9_LAB_SPARE_TABLES=1 DNBOX_MIAMI_DECIMATOR_COMPACT=1 DNBOX_CHICAGO_DRIVE_COMPACT=1 DNBOX_CHICAGO_MOD_SPEED_COMPACT=1
python3 "$P/link-guarded-exact.py" --board smk37 --model elite --sdk "$AC79_SDK"
