#!/bin/bash
set -euo pipefail
d=$(cd "$(dirname "$0")"&&pwd);r=$(dirname "$d");p=/tmp/phaser-warm-own.Y0XNbQ/tree
t="$r/prereqs/jieli-linux-toolchains-20260730.1/pi32v2";hw="$r/s1-native-909-ride-boundary"
o=$(mktemp -d "$d/bare.XXXX");trap 'rm -rf "$o"' EXIT
export TMPDIR="$o" PYTHONDONTWRITEBYTECODE=1
emu="$r/restored-gate/fm1-live-ba83317a"
core="$p/firmware/src/dsp/dnbx_core.c"
echo "fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017  $emu" |sha256sum -c -
echo "70d0aa0ea1cf2192e50cd51b45dffe01533cde368c0a1778959d39312b0ce759  $core" |sha256sum -c -
sha256sum "$core" "$d/full.c" "$hw/"{start.S,bench.ld}
echo "8bd49ada1e2cf9f56570038ad505cbc46b9917bfbe70a5f9fc1caec8aa85f610  $d/full.c" |sha256sum -c -
python3 "$d/compose.py" "$d/full.c" "$d/tangent-auth.py" "$o/tan-leaf.c"
echo "7a6599f3cf9912373f2d356ae67585db231b368705ff80d4c54321eba01de071  $d/tangent-auth.py" |sha256sum -c -
{
 printf '#include "fastmath.h"\n#include <string.h>\n#define tanhf fm_tanhf\n'
 sed -e '1{/^#include <math.h>$/d;}' -e '/__attribute__((visibility("default")))/,$d' "$o/tan-leaf.c"
 cat "$d/rollout.c.inc"
} >"$o/stock.c"
{
 printf '#include "fastmath.h"\n#include <string.h>\n#define tanhf fm_tanhf\n'
 sed -e '1{/^#include <math.h>$/d;}' -e '/__attribute__((visibility("default")))/,$d' "$d/full.c"
 cat "$d/rollout.c.inc"
} >"$o/candidate.c"
F=(-target pi32v2 -mcpu=r3 -mfprev1 -mllvm -fast-isel=false -ffp-contract=off -Os -fno-builtin -ffunction-sections -fdata-sections -isystem "$t/include" -I"$p/firmware/src/dsp" -I"$p/firmware/hal")
for arm in stock candidate;do
 flags=();[[ "$arm" != stock ]] || flags=(-DTANGENT=1)
 "$t/bin/clang" "${F[@]}" "${flags[@]}" -DFN=$arm -c "$o/$arm.c" -o "$o/$arm.o"
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
 "$t/bin/nm" -S --defined-only "$o/toy.elf" | grep -E 'stock|candidate|clock_2x|sj6_ota_filter'
 "$t/bin/objcopy" -O binary -j .text "$o/toy.elf" "$o/toy.bin"
 python3 -c 'from pathlib import Path;p=Path("toy.bin");p.with_suffix(".padded").write_bytes(bytes(0x120)+p.read_bytes())'
 FM1_STRICT=1 FM1_NO_SKIP=1 FM1_NO_PLAY=1 FM1_DUMP=0x01c10000 timeout 15 "$emu" --bin "$o/toy.padded" --entry 0x02000120 --mhz 360 --shot 900 --lcd-only >"$d/active-$n.log" 2>&1
 grep '^dump 0x1c10000:' "$d/active-$n.log"
done
