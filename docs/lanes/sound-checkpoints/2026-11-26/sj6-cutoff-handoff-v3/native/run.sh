#!/usr/bin/env bash
set -eu
D="$(cd "$(dirname "$0")"; pwd)"
R="$(cd "$D/../..";pwd)"
V="${1:?0 or 1}"
M="${2:?elite or pro}"
B="$D/$V-$M"
mkdir -p "$B"
for dir in firmware tools assets build experiments ble reference; do
 cp --reflink=auto -a "$R/s1-morph-native-nosplit/tree/$dir" "$B/"
done
cd "$B"
rm -f -- "$B/firmware/src/dsp/juno6_note_cutoff.h"
patch -s --fuzz=0 -p1 < "$D/../candidate.patch"
cat "$R/all-controls-remote-join/compile-flags.txt" > "$B/local-flags.txt"
printf '\n-DX0X_SJ6_NOTE_CUTOFF_LATCH=%s\n' "$V" >> "$B/local-flags.txt"
export JIELI_TOOLCHAIN="$R/prereqs/jieli-linux-toolchains-20260730.1" AC79_SDK="$R/prereqs/sdk"
export PYTHONPATH="$B" DNBOX_CONTROL_FLAGS="$B/local-flags.txt" D9_LAB_OUT="$B/out"
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1 X0X_ARP_HOOKS=1 X0X_REMOTE_DNR3=1 X0X_REMOTE_TYPED_SYNTH=1
export D9_LAB_XDATA_TAIL=1 D9_LAB_SLOT3_METADATA=1 DNBOX_TANH_SPLIT_LAB=1 DNBOX_TANH_REFILL_LAB=1 D9_LAB_S1_FDATA=1 DNBOX_CHICAGO_MOD_PALETTE=1 DNBOX_MIAMI_STOCK_CAP_COMPACT=1 D9_LAB_SPARE_TABLES=1 DNBOX_MIAMI_DECIMATOR_COMPACT=1 DNBOX_CHICAGO_DRIVE_COMPACT=1 DNBOX_CHICAGO_MOD_SPEED_COMPACT=1
python3 "$R/s1-morph-native-nosplit/link-guarded-exact.py" --board smk37 --model "$M" --sdk "$AC79_SDK"
