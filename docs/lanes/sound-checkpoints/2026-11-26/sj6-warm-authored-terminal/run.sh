#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd); R=$(dirname "$D")
W="${SJ6_STRICT_WARM_SOURCE:-/tmp/phaser-warm-own.Y0XNbQ}/tree"
P="$R/sj6-j60-right-native";B="$R/sj6-cross-preset"
O=$(mktemp -d "$D/scratch.XXXX"); trap 'rm -rf "$O"' EXIT
T="$O/tree"; mkdir "$T"
cp -a "$W/firmware" "$T/"
ln -s "$W/experiments" "$T/experiments"
cd "$T"
sha256sum firmware/src/{app/original_runtime.c,dsp/juno6.c,dsp/dnbx_core.c} >&2
sha256sum "$W/lab-cold-0-2/"{elite,pro}/never-install.elf >&2
for p in wet-lease table-share j60-post;do patch --fuzz=0 -s -p1 < "$P/$p.patch";done
python3 "$P/apply.py" firmware/src/dsp/juno6_j60_native.inc "$P/right-stretch-host.patch"
sha256sum firmware/src/{app/original_runtime.c,dsp/juno6_j60_native.inc,dsp/dnbx_core.c} >&2
export TMPDIR="$O"
# The host projection feeds the exact real donor typed owner and callbacks.
# Same inputs across the flags. Preserve source and range, not old host overlays.
for a in stock post both;do
 J=1; TR=0; if [[ "$a" == stock ]]; then J=0;fi; if [[ "$a" == both ]]; then TR=1;fi
 F=$(cat "$R/all-controls-remote-join/compile-flags.txt")
 cc -std=c99 -O2 -ffp-contract=off -ffunction-sections -fdata-sections -Wno-attributes \
 -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_ORIGINAL_SYNTH=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 \
 -DX0X_CHICAGO_FAST32=1 -DDNBOX_MIAMI_DECIMATOR_COMPACT=1 -DDNBOX_CHICAGO_MOD_SPEED_COMPACT=1 -DDNBOX_CHICAGO_DRIVE_COMPACT=1 -DDNBOX_CHICAGO_MOD_PALETTE=1 -DDNBOX_MIAMI_STOCK_CAP_COMPACT=1 \
 -DDNBOX_SHARED_LE32_LAB=1 -DDNBOX_S1_MATCHED_PHASER_LAB=1 -DDNBOX_S1_MATCHED_PHASER_F32=1 \
 -DDNBOX_S1_WET_LEASE_LAB=1 -DDNBOX_S1_WET_LEASE_USE=0 -DDNBX_EXPERIMENT_J60_POST=$J -DDNBX_EXPERIMENT_J60_RIGHT_TRAJECTORY=$TR $F \
 -I"$W/build/smk37-elite/gen" -Ifirmware/src/app -Ifirmware/src/dsp -Ifirmware/src -Iexperiments/original_synth -Iexperiments/wobble \
 "$B/render.c" "$B/foreign-fail-closed.c" "$D/foreign2.c" firmware/src/dsp/juno6.c firmware/src/dsp/dnbx_core.c -Wl,--gc-sections -lm -o "$O/probe-$a"
 for s in dry authored;do "$O/probe-$a" "$B/sub-range.private-output/2-$s.private.txt" "$O/$a-$s.f32" 2>"$O/$a-$s.log";done
done
OPENBLAS_NUM_THREADS=1 python3 "$D/score.py" "$O" "$B"
