#!/bin/bash
set -eu
: "${W:?pinned workspace root}"
[[ $PWD == /tmp/dnbox-emulator-only-* ]]
M=$(dirname "$(find "$W/s1-fx-parity/reference-build" -path '*/AudioKit/Core/Soundpipe/modules/revsc.c' -print -quit)")
H=$(dirname "$(dirname "$(find "$W/s1-fx-parity/reference-build/closure/experiment" -path '*/AudioKit/Core/Soundpipe/modules/buthp.c' -print -quit)")")
T="$W/prereqs/jieli-linux-toolchains-20260730.1";C="$T/common/bin/clang";L="$T/pi32v2/bin/ld"
FLAGS=(-target pi32v2 -mcpu=r3 -Os -ffp-contract=off -ffunction-sections -fdata-sections -I"$T/pi32v2/include" -I"$W/all-controls-source-join/tree/firmware/hal" -I"$H" -I"$H/lib/dr_wav" -DNO_LIBSNDFILE)
"$C" "${FLAGS[@]}" -c "$M/vdelay.c" -o vdelay.o
"$C" "${FLAGS[@]}" -c boot.S -o boot.o
for x in base new; do
 ADD=(); [ "$x" != new ] || ADD+=(-DPAIR_CUBIC)
 "$C" "${FLAGS[@]}" "${ADD[@]}" -c bench.c -o "$x.o"
 "$L" -T target.ld --gc-sections -o "$x.elf" boot.o "$x.o" vdelay.o --start-group "$T/pi32v2/lib/r3/libm.a" "$T/pi32v2/lib/r3/libc.a" "$T/pi32v2/lib/r3/libcompiler-rt.a" --end-group
 "$T/pi32v2/bin/objcopy" -O binary -j .text "$x.elf" "$x.bin"
done
PYTHONDONTWRITEBYTECODE=1 python seed.py
for x in base new;do cp "$x-nor-pristine" "$x-nor.bin";done
flock -n -E 79 /tmp/dnbox-gate-firmware.lock bash -c '
set -e
for x in base new;do
 env FM1_STRICT=1 FM1_NO_SKIP=1 FM1_DUMPRAM="$x.ram" timeout 16s "$1/restored-gate/fm1-live-ba83317a" --fwsc "$1/out/DnBOX-0.0.27-SMK37-elite-73749f55-test.fwsc" --flash "$PWD/$x-nor.bin" --mhz 360 --shot 1600 >"$x.log" 2>&1
 cmp "$x-nor.bin" "$x-nor-pristine"
done' -- "$W"
python read.py
wc -c *.bin
