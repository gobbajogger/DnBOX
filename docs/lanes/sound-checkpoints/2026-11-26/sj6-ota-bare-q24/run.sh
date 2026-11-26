#!/bin/bash
set -euo pipefail
d=$(cd "$(dirname "$0")"&&pwd);r=$(dirname "$d");p=/tmp/phaser-warm-own.Y0XNbQ/tree
t="$r/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"; hw="$r/s1-native-909-ride-boundary"; h="$r/sj6-ota-history"
o=$(mktemp -d "$d/bare.XXXX");trap 'rm -rf "$o"' EXIT;export TMPDIR="$o" PYTHONDONTWRITEBYTECODE=1
emu="$r/restored-gate/fm1-live-ba83317a"
echo "fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017  $emu" |sha256sum -c -
sha256sum "$d/"{full.c,transform.py} "$hw/"{start.S,bench.ld}
echo "8bd49ada1e2cf9f56570038ad505cbc46b9917bfbe70a5f9fc1caec8aa85f610  $d/full.c" |sha256sum -c -
echo "d724669d8755f14abe1b1e05311c1fb6d9cdc25c8854dc60022115204a1c6831  $d/transform.py" |sha256sum -c -
cp "$d/full.c" "$o/split-full.c"
python3 "$d/transform.py" "$d/full.c" "$o/q24-full.c" both
variant=${Q24_NOINLINE:-0}
case "$variant" in 0) mode=inline; hook=();; 1) mode=noinline; hook=(-e 's/^static inline float ap_q24/static __attribute__((noinline)) float ap_q24/');; *) exit 2;; esac
sed "${hook[@]}" -e 's/\bmemcpy(/__builtin_memcpy(/g' -e '/#include <string.h>/d' "$o/q24-full.c" >"$o/transposed-full.c"
F=(-target pi32v2 -mcpu=r3 -mfprev1 -mllvm -fast-isel=false -ffp-contract=off -Os -fno-builtin -ffunction-sections -fdata-sections -isystem "$t/include" -I"$p/experiments/original_synth" -I"$p/firmware/src/dsp" -I"$p/firmware/hal")
for arm in split transposed;do
  sed '/__attribute__((visibility("default")))/,$d' "$o/$arm-full.c" >"$o/$arm.c"
  cat "$d/rollout.c.inc" >>"$o/$arm.c"
  "$t/bin/clang" "${F[@]}" -Dtanhf=fm_tanhf -DFN=measure_$arm -c "$o/$arm.c" -o "$o/$arm.o"
done
"$t/bin/clang" "${F[@]}" -c "$d/main.c" -o "$o/main.o"
"$t/bin/clang" "${F[@]}" -c "$hw/start.S" -o "$o/boot.o"
"$t/bin/clang" "${F[@]}" -c "$d/tanh.c" -o "$o/tanh.o"
"$t/bin/ld" -T "$hw/bench.ld" --gc-sections -o "$o/toy.elf" "$o/boot.o" "$o/main.o" "$o/split.o" "$o/transposed.o" "$o/tanh.o" "$t/lib/r3/libcompiler-rt.a"
"$t/bin/objcopy" -O binary -j .text "$o/toy.elf" "$o/toy.bin"
python3 - "$o/toy.bin" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1]);p.with_suffix('.padded').write_bytes(bytes(0x120)+p.read_bytes())
PY
for arm in split transposed;do
 "$t/bin/nm" -u "$o/$arm.o"
 "$t/bin/nm" -S --defined-only "$o/$arm.o"
 readelf -SW "$o/$arm.o"
done | tee "$d/$mode-object.log"
! grep -q 'fma' "$d/$mode-object.log"
"$t/bin/nm" -S --defined-only "$o/toy.elf" | grep -E 'measure_|clock_2x|ap_q24'
cd "$o"
mkdir work
exec 9>/tmp/dnbox-gate-firmware.lock
flock -n -E 79 -x 9
FM1_STRICT=1 FM1_NO_SKIP=1 FM1_NO_PLAY=1 FM1_DUMP=0x01c10000 timeout 15 "$emu" --bin "$o/toy.padded" --entry 0x02000120 --mhz 360 --shot 900 --lcd-only >"$d/$mode-bare.log" 2>&1
