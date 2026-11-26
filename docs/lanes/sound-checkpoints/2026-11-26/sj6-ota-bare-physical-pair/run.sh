#!/bin/bash
set -euo pipefail
d=$(cd "$(dirname "$0")"&&pwd);r=$(dirname "$d");p=/tmp/phaser-warm-own.Y0XNbQ/tree
t="$r/prereqs/jieli-linux-toolchains-20260730.1/pi32v2";hw="$r/s1-native-909-ride-boundary"
o=$(mktemp -d "$d/bare.XXXX");trap 'rm -rf "$o"' EXIT
export TMPDIR="$o" PYTHONDONTWRITEBYTECODE=1
emu="$r/restored-gate/fm1-live-ba83317a"
echo "fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017  $emu" |sha256sum -c -
echo "703d2534814e4169b1f801cf1dc026317bffcbdda209ad9c0647d239f2b97ea0  $d/SJ6-full-physical-paired-source.tgz" |sha256sum -c -
for arm in stock pair; do
 tar --warning=no-unknown-keyword -xOzf "$d/SJ6-full-physical-paired-source.tgz" "$arm.c" | cmp - "$d/$arm.c"
done
for arm in stock candidate;do
 origin="$arm"; [[ "$arm" != candidate ]] || origin=pair
 { printf '#include "fastmath.h"\n#include <string.h>\n#define tanhf fm_tanhf\n'; sed -e '1{/^#include <math.h>$/d;}' -e '/__attribute__((visibility("default")))/,$d' "$d/$origin.c"; cat "$d/rollout.c.inc"; } >"$o/$arm.c"
done
F=(-target pi32v2 -mcpu=r3 -mfprev1 -mllvm -fast-isel=false -ffp-contract=off -Os -fno-builtin -ffunction-sections -fdata-sections -isystem "$t/include" -I"$p/firmware/src/dsp" -I"$p/firmware/hal")
for arm in stock candidate;do
 flags=(); [[ "${PREPARED:-0}" != 1 ]] || flags=(-DPREPARED=1)
 "$t/bin/clang" "${F[@]}" "${flags[@]}" -DFN=$arm -Dprepare_tile=prepare_tile_$arm -c "$o/$arm.c" -o "$o/$arm.o"
 "$t/bin/nm" -u "$o/$arm.o"
done
"$t/bin/clang" "${F[@]}" -c "$hw/start.S" -o "$o/boot.o"
exec 9>/tmp/dnbox-gate-firmware.lock
flock -n -E 79 -x 9
cd "$o"
mkdir work
for n in 1 4 6;do
 "$t/bin/clang" "${F[@]}" -DACTIVE=$n -c "$d/main.c" -o "$o/main.o"
 "$t/bin/ld" -T "$hw/bench.ld" --gc-sections -o "$o/toy.elf" "$o/boot.o" "$o/main.o" "$o/stock.o" "$o/candidate.o" "$t/lib/r3/libcompiler-rt.a"
 "$t/bin/nm" -S --defined-only "$o/toy.elf" | grep -E 'stock|candidate|clock_2x|clock_pair|prepare_tile'
 "$t/bin/objcopy" -O binary -j .text "$o/toy.elf" "$o/toy.bin"
 python3 -c 'from pathlib import Path;p=Path("toy.bin");p.with_suffix(".padded").write_bytes(bytes(0x120)+p.read_bytes())'
 FM1_STRICT=1 FM1_NO_SKIP=1 FM1_NO_PLAY=1 FM1_DUMP=0x01c10000 timeout 15 "$emu" --bin "$o/toy.padded" --entry 0x02000120 --mhz 360 --shot 900 --lcd-only >"$d/active-$n.log" 2>&1
 grep '^dump 0x1c10000:' "$d/active-$n.log"
done
