#!/bin/sh
set -eu
cd "$(dirname "$0")"
D=$(mktemp -d)
trap 'rm -r "$D"' EXIT
BASE=$(realpath "${1:-../all-controls-source-join/tree}")
mkdir -p "$D/firmware"
cp -R "$BASE/firmware/src" "$D/firmware/"
ln -s "$BASE/experiments" "$D/experiments"
(cd "$D" && patch -s --fuzz=0 -p1) < candidate.patch
T="$D/firmware/src"
CFLAGS="-std=c99 -O2 -ffp-contract=off -ffunction-sections -fdata-sections -Wno-attributes -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_LFO1_DELAY_PORT=1 -DX0X_SJ6_AUX_LFO_NATIVE=1 -DX0X_SJ6_OTA_NATIVE=1"
cc $CFLAGS -I"$T/dsp" -I"$T/app" -I"$T" -DX0X_SJ6_NOTE_CUTOFF_LATCH=1 probe.c "$T/dsp/juno6.c" "$T/dsp/dnbx_core.c" -Wl,--gc-sections -lm -o "$D/probe"
"$D/probe"
# The recovered snapshot lacks a contemporary original_sound.h. Only the host
# fixture overlays that header; no donor headers are included in the patch.
H=$(realpath "${2:-../s1-morph-native-nosplit/tree/firmware/src/app/original_sound.h}")
printf 'de94cce9176397b68773b9c450efc0c58e9c7228160f919b6e85ee65e7eac83f  %s\n' "$H" | sha256sum -c >/dev/null
cp "$H" "$T/app/original_sound.h"
cc $CFLAGS -Werror=implicit-function-declaration -I"$T/dsp" -I"$T/app" -I"$T" -I"$D/experiments/original_synth" -DX0X_NOTE_CUTOFF_PROBE=1 -DX0X_SJ6_NOTE_CUTOFF_LATCH=1 -DX0X_SJ6_LFO_TRIGGER_NATIVE=1 owner-probe.c unused-links.c "$T/dsp/juno6.c" "$T/dsp/dnbx_core.c" "$T/app/original_runtime.c" -Wl,--gc-sections -lm -o "$D/owner-probe"
"$D/owner-probe"
for opt in 0 1; do
  cc $CFLAGS -I"$T/dsp" -I"$T/app" -I"$T" -DX0X_SJ6_NOTE_CUTOFF_LATCH="$opt" dormant.c "$T/dsp/juno6.c" "$T/dsp/dnbx_core.c" -Wl,--gc-sections -lm -o "$D/guard"
  "$D/guard" > "$D/dormant.$opt.f32"
done
cmp "$D/dormant.0.f32" "$D/dormant.1.f32"
sha256sum "$D/dormant.0.f32" | cut -d' ' -f1
echo "dormant no-latch PCM bit-identical (10,240 words)"
