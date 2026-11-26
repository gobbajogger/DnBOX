#!/bin/bash
set -euo pipefail
R=$(cd "$(dirname "$0")/../.."&&pwd); D="$R/sj6-raw-main-ota"; X="$R/sj6-ota-history/lease-connected"
W="${SJ6_STRICT_WARM_SOURCE:-/tmp/phaser-warm-own.Y0XNbQ}/tree"
P="$R/sj6-j60-right-native";B="$R/sj6-cross-preset"
O=$(mktemp -d "$X/scratch.XXXX"); trap 'rm -rf "$O"' EXIT
T="$O/tree"; mkdir "$T"
cp -as "$W/firmware" "$T/"
for p in wet-lease table-share j60-post; do
 while read -r file; do
   [[ "$file" == a/* ]] || continue
   file=${file#a/}; rm -- "$T/$file"; cp "$W/$file" "$T/$file"
 done < <(awk '/^--- /{print $2}' "$P/$p.patch")
done
ln -s "$W/experiments" "$T/experiments"
cd "$T"
sha256sum firmware/src/{app/original_runtime.c,dsp/juno6.c,dsp/dnbx_core.c} >&2
sha256sum "$W/lab-cold-0-2/"{elite,pro}/never-install.elf >&2
for p in wet-lease table-share j60-post;do patch --fuzz=0 -s -p1 < "$P/$p.patch";done
python3 "$P/apply.py" firmware/src/dsp/juno6_j60_native.inc "$P/right-stretch-host.patch"
sha256sum firmware/src/{app/original_runtime.c,dsp/juno6_j60_native.inc,dsp/dnbx_core.c} >&2
export TMPDIR="$O"
python3 "$R/sj6-j60-warm-center/compose.py" firmware/src/dsp/dnbx_core.c firmware/src/dsp/juno6_j60_native.inc "$B/render.c" "$O"
python3 "$B/meter.py" firmware/src/app/original_runtime.c "$O/runtime.c"
# The host projection feeds the exact real donor typed owner and callbacks.
# Same inputs across the flags. Preserve source and range, not old host overlays.
python3 "$D/wire.py" "$O/core.c" "$O/driver.c"
for q in dnbx_core.h juno6.h juno6.c; do rm -- "firmware/src/dsp/$q"; cp "$W/firmware/src/dsp/$q" "firmware/src/dsp/$q"; done
python3 "$R/sj6-stock-layout/wire-gaps.py" firmware/src/dsp/juno6.h firmware/src/dsp/juno6.c --retain-gaps
patch --fuzz=0 -p1 -s < "$R/sj6-ota-history/pitchenv-lease/pitch-layout.patch"
python3 "$R/sj6-ota-history/transposed-auth/upstream.py" "$D/physical.c" "$O/fused.c"
sed -E 's/fmaf\(-a,mid,x\)/x-a*mid/;s/fmaf\(-b,y,mid\)/mid-b*y/' "$O/fused.c" > "$O/separate.c"
python3 "$R/sj6-ota-history/hoist/wire.py" "$O/separate.c" "$O/transposed.inc"
python3 "$X/tripwire.py" "$O/core.c"
python3 "$X/wire.py" "$O/core.c" "$O/driver.c" firmware/src/dsp/dnbx_core.h firmware/src/dsp/juno6.h "$O"

 J=1; TR=1; C=1
 F=$(cat "$R/all-controls-remote-join/compile-flags.txt")
 cc -shared -fPIC -fvisibility=hidden -I"$D" -std=c99 -O2 -ffp-contract=off -ffunction-sections -fdata-sections -Wno-attributes \
 -I"$O" -DDNBX_SJ6_RECLAIM_STOCK_LAYOUT=1 -DDNBX_DIAGNOSTIC_SJ6_PITCH_LEASE=1 -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_ORIGINAL_SYNTH=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 \
 -DX0X_CHICAGO_FAST32=1 -DDNBOX_MIAMI_DECIMATOR_COMPACT=1 -DDNBOX_CHICAGO_MOD_SPEED_COMPACT=1 -DDNBOX_CHICAGO_DRIVE_COMPACT=1 -DDNBOX_CHICAGO_MOD_PALETTE=1 -DDNBOX_MIAMI_STOCK_CAP_COMPACT=1 \
 -DDNBOX_SHARED_LE32_LAB=1 -DDNBOX_S1_MATCHED_PHASER_LAB=1 -DDNBOX_S1_MATCHED_PHASER_F32=1 \
 -DDNBOX_S1_WET_LEASE_LAB=1 -DDNBOX_S1_WET_LEASE_USE=0 -DDNBX_EXPERIMENT_J60_POST=$J -DDNBX_EXPERIMENT_J60_RIGHT_TRAJECTORY=$TR -DDNBX_HOST_SJ6_AUTHENTIC_CENTER=$C "-DSJ6_RUNTIME_OVERRIDE=\"$O/runtime.c\"" $F \
 -I"$W/build/smk37-elite/gen" -Ifirmware/src/app -Ifirmware/src/dsp -Ifirmware/src -Iexperiments/original_synth -Iexperiments/wobble \
 "$O/driver.c" "$B/foreign-fail-closed.c" "$R/sj6-j60-warm-center/foreign2.c" firmware/src/dsp/juno6.c "$O/core.c" -Wl,--gc-sections -lm -o "$O/combined.so"
cd "$R"
OPENBLAS_NUM_THREADS=1 PYTHONPATH=/tmp/sj6-owner-packages:"$R/ab-effects-readonly/sj6-fullbench/bench" python3 "$X/score.py" "$O" "$B"
OPENBLAS_NUM_THREADS=1 PYTHONPATH=/tmp/sj6-owner-packages:"$R/ab-effects-readonly/sj6-fullbench/bench" python3 "$X/reject.py" "$O" > "$X/reject.json"
