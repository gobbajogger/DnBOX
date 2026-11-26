#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D"); A="$R/recovered-sound/s1-room-parity"
C=$(realpath "$1"); T=$(realpath "$2"); SRC="$R/ab-latest-headless"
MOD=$(find "$R/s1-fx-parity/reference-build" -path '*/AudioKit/Core/Soundpipe/modules/revsc.c' -print -quit)
M=$(dirname "$MOD")
HP=$(find "$R/s1-fx-parity/reference-build/closure/experiment" -path '*/AudioKit/Core/Soundpipe/modules/buthp.c' -print -quit)
H=$(dirname "$(dirname "$HP")")
L="$R/restored-handoff/docs/handoffs/2026-11-24-source/composed-controls"
read -ra opts <<<"$(grep -o -- '-DX0X_S1[A-Z0-9_]*=1' "$L/FLAGS.txt"|xargs)"
extra=()
case "$C" in
  */build5) ;;
  */build-filter)
    sha256sum -c <(printf '%s  %s\n' \
       bc353df5230c2be69a238594cd8e03ee53b0578807876cd7cfe40312bcc78959 "$C/dsp/s1c/s1c.c" \
       1e1cdf4acea0d0603fa089fc8fa0e1e60cdd388616c44d84d979c741f4b69c15 "$C/render.c" \
       84c2c5c73541302cb4d9d454f02693c293127bc68a3a74e003d119f68c42fc1d "$C/part.c")
    extra+=(-DS1_NOISE_EXACT=1 -DS1_FILTER_EXACT=1 "$A/exact-filter.c" "$H/modules/butbp.c")
    ;;
  */build-morph)
    sha256sum -c <(printf '%s  %s\n' \
       34005f1d1fa14c3267bf8dc7e9f3d0d437dd9f65d2db8c7c72082bd9bf59cb86 "$C/dsp/s1c/s1c.c" \
       b740376002a86c23bca6e0fbdcf75cbefa7c42dd128e3a2a21bcdfae24daf7f4 "$C/render.c" \
       297bd754a9b3c515c7a77be0b9ce7ea761067e748c3344419291a94afa8b26a4 "$C/part.c")
    extra+=(-DS1_NOISE_EXACT=1 -DS1_FILTER_EXACT=1 -DS1_MORPH_EXACT=1 "$A/exact-filter.c" "$H/modules/butbp.c" "$A/exact-morph.c")
    ;;
  *) echo "Unpinned donor: $C" >&2; exit 3 ;;
esac
cd "$SRC"
cc -std=gnu11 -O2 -ffunction-sections -fdata-sections -include "$C/dsp/s1c/s1c.h" -DS1_WIDEN_ORACLE=1 -DS1_ROOM_EXACT=1 -DS1_ROOM_EXACT_MASTER=1 -DS1_DELAY_EXACT=1 \
     -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 "${opts[@]}" -DX0X_SUPERJ6_BUILTIN_INIT=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 \
     -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_FEEDBACK_PROBE=1 \
     -DORIGINAL_USE_SOFT_FMA -ffp-contract=off -fno-fast-math -DNO_LIBSNDFILE -I"$A" -I"$M/../lib/faust" -I"$H" -I"$H/lib/dr_wav" \
     -I"$C/dsp/s1c" -I"$C/dsp" -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
     "$T/render.c" "$C/part.c" "$T/s1c.c" "$C/dsp/s1c/"{s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c \
     "${extra[@]}" "$A/"{exact-room,exact-delay}.c "$M/"{revsc,compressor,vdelay}.c "$H/modules/"{buthp,base,crossfade,moogladder}.c -Wl,--gc-sections -lm -o "$T/candidate"
