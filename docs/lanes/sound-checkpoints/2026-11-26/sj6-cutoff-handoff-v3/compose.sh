#!/bin/sh
set -eu
cd "$(dirname "$0")"
D=$(mktemp -d)
trap 'rm -r "$D"' EXIT
W=$(realpath "${1:-..}")
BASE="$W/all-controls-source-join/tree"
M="$W/recovered-sound/main-route"
P="$W/recovered-sound/sj6-handoff-clock"
T="$D/firmware/src"
hash() { printf '%s  %s\n' "$1" "$2" | sha256sum -c >/dev/null; }
hash 62434953c3d95d365f36f307c7a6a8d8edddf3d20dea864720ea0a19a2fcae85 "$M/exact-feature/juno6_factory.inc"
hash bd5a59c1b46fdf65f7d4e8eb0d9780fcbf21841906d00a672309a4ee20532f76 "$M/dnbx_core.c"
hash 40e9a50b14cc90e34fee6bccb85d222c6f1865c55e4749967915cf684bb03385 "$M/exact-feature/dnbx_core.h"
hash 69d1fb6f971427aecda1f06d1ff7770aa1d77c299ad335d748392a391075fe98 "$M/exact-feature/juno6.c"
hash fe9406077bd1195b3e95a5ca3c78b52e9c9838119ac6b1039ec71f6c5bb6df14 "$M/exact-feature/juno6.h"
hash 00d63b35fc84c108f2ce46b1b039af943f6cceeafa9f51f2e5461e0bf4d2347c "$M/exact-feature/replay_source_bus_tap.c"
mkdir -p "$D/firmware"
cp -R "$BASE/firmware/src" "$D/firmware/"
ln -s "$BASE/experiments" "$D/experiments"
cp "$M"/exact-feature/juno6_factory.inc "$M"/exact-feature/juno6.c "$M"/exact-feature/juno6.h "$M"/exact-feature/dnbx_core.h "$M/dnbx_core.c" "$T/dsp/"
cp "$M"/exact-feature/replay_source_bus_tap.c "$T/dsp/"
(cd "$T/dsp" && patch -s --fuzz=0 -p1) < "$P/release-glide-host.patch"
(cd "$T/dsp" && patch -s --fuzz=0 -p1) < "$P/trace-only.patch"
(cd "$D" && patch -s --fuzz=0 -p1) < candidate.patch
F="-std=c99 -O2 -ffp-contract=off -ffunction-sections -fdata-sections -Wno-attributes -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NOTE_CUTOFF_LATCH=1 -DX0X_SJ6_AUX_LFO_NATIVE=1 -DX0X_SJ6_LFO1_DELAY_PORT=1 -DX0X_SJ6_OTA_NATIVE=1 -DX0X_SJ6_MONO32_PROBE=1 -DDNBX_HOST_MAIN_SOURCE_ORACLE=1 -DDNBOX_HOST_RELEASE_GLIDE_WITNESS=1"
cc $F -I"$T/dsp" -I"$M" "$T/dsp/replay_source_bus_tap.c" "$T/dsp/juno6.c" "$T/dsp/dnbx_core.c" -Wl,--gc-sections -lm -o "$D/phase"
"$D/phase" "$D/three-main.private.f32" 2>"$D/stages"
hash f475ce83d3ceae6413669fd74c8a69e615c2ec77e4a0113b4f8792a12d102d95 "$D/three-main.private.f32"
echo PASS: expected release-anchor source SHA, disabled cutoff arm byte-identical
H="$W/s1-morph-native-nosplit/tree/firmware/src/app/original_sound.h"
hash de94cce9176397b68773b9c450efc0c58e9c7228160f919b6e85ee65e7eac83f "$H"
cp "$H" "$T/app/original_sound.h"
cc $F -Werror=implicit-function-declaration -I"$T/dsp" -I"$T/app" -I"$T" -I"$M" -I"$D/experiments/original_synth" timed-owner.c unused-links.c "$T/dsp/juno6.c" "$T/dsp/dnbx_core.c" "$T/app/original_runtime.c" -Wl,--gc-sections -lm -o "$D/timed"
"$D/timed"
cc $F -I"$T/dsp" -I"$M" arbitration.c "$T/dsp/juno6.c" "$T/dsp/dnbx_core.c" -Wl,--gc-sections -lm -o "$D/arb"
"$D/arb"
