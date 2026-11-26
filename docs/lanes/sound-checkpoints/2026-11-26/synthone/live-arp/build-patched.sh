#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D"); cd "$R"
T=s1-organ-wet-recovery/native-widen-bfp/tree
P="$D/private/full-tree"; Q="$P/firmware/src"
test ! -e "$Q" || { echo "Move aside scratch full-tree before fresh rebuild"; exit 3; }
python "$D/make-patch.py"
mkdir -p "$P/firmware" "$D/private/patched-include"
cp -a "$T/firmware/src" "$Q"
patch -s -p1 -d "$P" < "$D/factory-arp-ingress.patch"
python "$D/extract.py" --patched
read -ra flags <<<"$(xargs < all-controls-remote-join/compile-flags.txt)"
common=(-DX0X_HOST -DX0X_SUPERJ6_FACTORY=1 -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1_FACTORY_ARP_INGRESS=1 "${flags[@]}" -I"$Q" -I"$Q/app" -I"$Q/dsp" -I"$Q/dsp/s1c" -I"$Q/dsp/synthone/sequencer" -I"$T/experiments/original_synth" -I"$T/experiments/wobble" -I"$T/build/smk37-elite/gen")
cc -std=gnu11 -O2 -ffp-contract=off -ffunction-sections -fdata-sections "${common[@]}" -Is1-fx-parity -I"$D/private/patched-include" "$D/emit.c" "$Q/dsp/s1c/s1c.c" -Wl,--gc-sections -lm -o "$D/private/emit-patched"
cc -std=gnu11 -O2 -ffp-contract=off -ffunction-sections -fdata-sections "${common[@]}" -DS1_ARP_REAL_PACK=1 -Is1-fx-parity -I"$D/private/patched-include" "$D/emit.c" "$Q/dsp/s1c/"{s1c,s1u,s1c_front}.c "$Q/app/s1c_part.c" "$Q/dsp/s1pack/s1pack.c" -Wl,--gc-sections -lm -o "$D/private/emit-packed"
cc -std=gnu11 -fsyntax-only "${common[@]}" "$Q/app/original_runtime.c" "$Q/app/engine.c"
RSEQ=s1-fx-parity/reference-build/closure/scheduler/source-sequence
g++ -std=c++17 -O2 -ffp-contract=off -I"$RSEQ" -Is1-fx-parity "$D/orig-events.cpp" -o "$D/private/orig-events"
