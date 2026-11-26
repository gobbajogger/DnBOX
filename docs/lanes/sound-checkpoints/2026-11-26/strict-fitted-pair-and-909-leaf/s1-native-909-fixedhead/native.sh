#!/bin/bash
set -euo pipefail
d=$(cd "$(dirname "$0")";pwd);r=$(dirname "$d")
t="$r/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"; s="$r/ui-native-table-share/tree/firmware/src"; gen="$r/ui-native-table-share/tree/build/smk37-elite/gen"
F=(-target pi32v2 -mllvm -fast-isel=false -mcpu=r3 -mfprev1 -Os -fno-builtin -ffp-contract=off -ffunction-sections -fdata-sections -idirafter "$t/include")
w=$(mktemp -d /tmp/dnbox-909fixed.XXXX);trap 'rm -r "$w"' EXIT
"$t/bin/clang" "${F[@]}" -c "$d/start.S" -o "$w/start.o"
for k in base candidate;do
 cat "$d/$k.c" "$d/bench-tail.inc" > "$w/$k.c"
 "$t/bin/clang" "${F[@]}" -DX0X_909_HUFF_DIAG=1 -DX0X_COLD_COMPANION_LAB=1 -DX0X_TANH_SPLIT_LAB=1 -I"$s/dsp" -I"$s" -I"$gen" -I"$r/fw/firmware/hal" -c "$w/$k.c" -o "$w/$k.o"
 "$t/bin/ld" --gc-sections -T "$d/bench.ld" "$w/start.o" "$w/$k.o" -o "$w/$k.elf"
 "$t/bin/objcopy" -O binary -j .text "$w/$k.elf" "$w/$k.bin"
 python - "$w/$k.bin" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1]);p.with_suffix('.padded').write_bytes(bytes(0x120)+p.read_bytes())
PY
done
mkdir "$w/work"; cd "$w"
exec 9>/tmp/dnbox-gate-firmware.lock
for k in base candidate;do
 flock -x 9
 FM1_STRICT=1 FM1_NO_SKIP=1 FM1_NO_PLAY=1 FM1_DUMPRAM="$w/$k.ram" "$r/restored-gate/fm1-live-ba83317a" --bin "$w/$k.padded" --entry 0x02000120 --mhz 360 --shot 370 >"$d/$k-native.log" 2>&1
 flock -u 9
 cp "$d/$k-native.log" "$w/$k.log"
done
python "$d/score.py"
