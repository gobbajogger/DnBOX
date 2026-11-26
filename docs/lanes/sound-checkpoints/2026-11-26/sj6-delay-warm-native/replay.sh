#!/bin/bash
# Diagnostic only. Running file paths must be a temporary emulator-only directory.
set -euo pipefail
export W=${W:?verified workspace root required}
[[ "$PWD" == /tmp/dnbox-emulator-only-* ]]
T="$W/prereqs/jieli-linux-toolchains-20260730.1"
EMU="$W/restored-gate/fm1-live-ba83317a"
echo "fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017  $EMU" | sha256sum -c -
mkdir -p work
for x in "$@"; do
 [[ "$x" == base || "$x" == new || "$x" == opt ]]
 "$T/common/bin/clang" -target pi32v2 -mcpu=r3 -Os -ffp-contract=off -ffunction-sections -fdata-sections -I"$T/pi32v2/include" -I"$W/all-controls-source-join/tree/firmware/hal" -I"$x" -c main.c -o "$x.o"
 "$T/common/bin/clang" -target pi32v2 -mcpu=r3 -c boot.S -o boot.o
 "$T/pi32v2/bin/ld" -T target.ld --gc-sections -o "$x.elf" boot.o "$x.o" --start-group "$T/pi32v2/lib/r3/libm.a" "$T/pi32v2/lib/r3/libc.a" "$T/pi32v2/lib/r3/libcompiler-rt.a" --end-group
 "$T/pi32v2/bin/objcopy" -O binary -j .text "$x.elf" "$x.bin"
done
PYTHONDONTWRITEBYTECODE=1 python seed.py "$@"
for x in "$@"; do
 cp "$x-nor-pristine" "$x-nor.bin"
 flock -n -E 79 /tmp/dnbox-gate-firmware.lock env FM1_STRICT=1 FM1_NO_SKIP=1 FM1_DUMPRAM="$x.ram" timeout 12s "$EMU"    --fwsc "$W/out/DnBOX-0.0.27-SMK37-elite-73749f55-test.fwsc" --flash "$PWD/$x-nor.bin" --mhz 360 --shot 1600 >"$x.log" 2>&1
done
python read.py "$@"
