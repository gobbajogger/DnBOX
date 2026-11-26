#!/bin/bash
set -euo pipefail
d=$(cd "$(dirname "$0")"&&pwd);r=$(dirname "$d");p=/tmp/phaser-warm-own.Y0XNbQ/tree
t="$r/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"; hw="$r/s1-native-909-ride-boundary"; h="$r/sj6-ota-history"
o=$(mktemp -d "$d/bare.XXXX");trap 'rm -rf "$o"' EXIT;export TMPDIR="$o" PYTHONDONTWRITEBYTECODE=1
emu="$r/restored-gate/fm1-live-ba83317a"
echo "fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017  $emu" |sha256sum -c -
for f in "$hw"/{start.S,bench.ld} "$r/sj6-raw-main-ota/physical.c" "$h"/{wire.py,exact-share.h,tanhreuse/fast.h,transposed-auth/upstream.py,hoist/wire.py};do sha256sum "$f";done
python3 "$h/transposed-auth/upstream.py" "$r/sj6-raw-main-ota/physical.c" "$o/transpose-fused.c"
sed -E 's/fmaf\(-a,mid,x\)/x-a*mid/;s/fmaf\(-b,y,mid\)/mid-b*y/' "$o/transpose-fused.c" >"$o/transpose.c"
python3 "$h/hoist/wire.py" "$o/transpose.c" "$o/split-full.c"
{
 printf '#undef fmaf\n#define fmaf(a,b,c) (((a)*(b))+(c))\n'
 cat "$o/split-full.c"
} >"$o/transposed-full.c"
F=(-target pi32v2 -mcpu=r3 -mfprev1 -mllvm -fast-isel=false -ffp-contract=off -Os -fno-builtin -ffunction-sections -fdata-sections -isystem "$t/include" -I"$p/experiments/original_synth" -I"$p/firmware/src/dsp" -I"$p/firmware/hal")
for arm in split transposed;do
  sed '/__attribute__((visibility("default")))/,$d' "$o/$arm-full.c" >"$o/$arm.c"
  cat "$d/rollout.c.inc" >>"$o/$arm.c"
  "$t/bin/clang" "${F[@]}" -DEXTERNAL_TANH=1 -DFN=measure_$arm -include "$h/exact-share.h" -include "$h/tanhreuse/fast.h" -c "$o/$arm.c" -o "$o/$arm.o"
done
"$t/bin/clang" "${F[@]}" -c "$d/main.c" -o "$o/main.o"
"$t/bin/clang" "${F[@]}" -c "$hw/start.S" -o "$o/boot.o"
"$t/bin/clang" "${F[@]}" -c "$p/experiments/original_synth/fused_reference.c" -o "$o/round.o"
"$t/bin/clang" "${F[@]}" -c "$d/tanh.c" -o "$o/tanh.o"
"$t/bin/ld" -T "$hw/bench.ld" --gc-sections -o "$o/toy.elf" "$o/boot.o" "$o/main.o" "$o/split.o" "$o/transposed.o" "$o/round.o" "$o/tanh.o" "$t/lib/r3/libcompiler-rt.a"
"$t/bin/objcopy" -O binary -j .text "$o/toy.elf" "$o/toy.bin"
python3 - "$o/toy.bin" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1]);p.with_suffix('.padded').write_bytes(bytes(0x120)+p.read_bytes())
PY
"$t/bin/nm" -u "$o/transposed.o" | tee "$d/separate-undefined.log"
! grep -q 'fma' "$d/separate-undefined.log"
"$t/bin/nm" -S --defined-only "$o/toy.elf" | grep -E 'measure_|clock_2x'
cd "$o"
mkdir work
exec 9>/tmp/dnbox-gate-firmware.lock
flock -n -E 79 -x 9
FM1_STRICT=1 FM1_NO_SKIP=1 FM1_NO_PLAY=1 FM1_DUMP=0x01c10000 timeout 15 "$emu" --bin "$o/toy.padded" --entry 0x02000120 --mhz 360 --shot 900 --lcd-only >"$d/bare.log" 2>&1
