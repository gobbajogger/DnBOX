#!/bin/sh
set -eu
d=$(realpath "$(dirname "$0")"); root=$(realpath "$1"); src=$(realpath "$2")
tc="$root/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
w=$(mktemp -d "$root/.s1pan-native.XXXXXX")
trap 'rm -rf "$w"' EXIT HUP INT TERM
mkdir -p "$w/firmware/src/dsp"
cp -a "$src/firmware/src/dsp/fastmath.h" "$src/firmware/src/dsp/s1c" "$w/firmware/src/dsp/"
cd "$w"
patch --fuzz=0 -s -p1 < "$d/autopan-port.patch"
for u in s1u s1c_front; do
  "$tc/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -Os -ffp-contract=off -fno-builtin \
   -ffunction-sections -fdata-sections -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1_AUTOPAN_PORT=1 \
   -DX0X_S1_AUTOPAN_PROBE=1 -DS1C_FASTMATH=1 -I firmware/src -idirafter "$tc/include" -c "firmware/src/dsp/s1c/$u.c" -o "$u.o"
  echo "$u"; "$tc/bin/nm" -u "$u.o"; readelf -SW "$u.o" | grep -E 'autopan_authored|front_pan_|pan_(boundary|detent)' | grep -v 'rela'
done | tee objects.log
! grep -Eq '^\s+U (lrintf|powf|__divdf3)' objects.log
