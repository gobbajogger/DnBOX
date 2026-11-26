#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D")
M=${1:?model}
BASE="$R/sj6-hp-exact-exp-fit/kept-$M-1"; B=$(mktemp -d "$D/tree.$M.XXXX")
trap 'rm -rf "$B"' EXIT
mkdir -p "$D/out/$M"
for p in assets reference ble;do ln -s "$BASE/$p" "$B/$p";done
cp -al "$BASE/experiments" "$B/"
for p in firmware tools build;do cp -a --reflink=auto "$BASE/$p" "$B/";done
cp "$BASE/link-with-inlet.py" "$BASE/flags.txt" "$B/"
cd "$B"
sha256sum firmware/src/dsp/juno6_j60_native.inc "$BASE/link-with-inlet.py" "$BASE/flags.txt"
patch -s --fuzz=0 -p1 < "$R/sj6-j60-wet-native/experimental.patch"
python3 "$D/apply.py" firmware/src/dsp/juno6_j60_native.inc "$R/sj6-j60-read/right-stretch-host.patch"
export JIELI_TOOLCHAIN="$R/prereqs/jieli-linux-toolchains-20260730.1" AC79_SDK="$R/prereqs/sdk"
export PYTHONPATH="$B" DNBOX_CONTROL_FLAGS="$B/flags.txt" D9_LAB_OUT="$B/out" TMPDIR="$B"
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1 X0X_ARP_HOOKS=1 X0X_REMOTE_DNR3=1 X0X_REMOTE_TYPED_SYNTH=1
export D9_LAB_XDATA_TAIL=1 D9_LAB_SLOT3_METADATA=1 DNBOX_TANH_SPLIT_LAB=1 DNBOX_TANH_REFILL_LAB=1 D9_LAB_S1_FDATA=1 DNBOX_CHICAGO_MOD_PALETTE=1 DNBOX_MIAMI_STOCK_CAP_COMPACT=1 D9_LAB_SPARE_TABLES=1 DNBOX_MIAMI_DECIMATOR_COMPACT=1 DNBOX_CHICAGO_DRIVE_COMPACT=1 DNBOX_CHICAGO_MOD_SPEED_COMPACT=1
for V in 0 1; do
 cp "$BASE/flags.txt" flags.txt
 printf '\n-DDNBX_EXPERIMENT_J60_POST=1 -DDNBX_EXPERIMENT_J60_RIGHT_TRAJECTORY=%s\n' "$V" >> flags.txt
 python3 link-with-inlet.py --board smk37 --model "$M" --sdk "$AC79_SDK"
 O="$D/out/$M/$V";mkdir -p "$O"
 cp "out/$M/"{never-install.elf,lab.map,text.bin} "$O/"
 find build -iname '*juno6*.o' -print -exec sha256sum {} \;
done
