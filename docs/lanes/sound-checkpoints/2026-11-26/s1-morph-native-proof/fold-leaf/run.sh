#!/bin/bash
set -euo pipefail
d=$(cd "$(dirname "$0")" && pwd); r=$(dirname "$d")
t="$r/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
f=(-target pi32v2 -mllvm -fast-isel=false -mcpu=r3 -mfprev1 -Os -fno-builtin -ffp-contract=off -ffunction-sections -fdata-sections -idirafter "$t/include")
defines=(-DDNBX_MORPH_TARGET_FRAGMENT -DDNBX_MORPH_PAIR=1 -DDNBX_MORPH_SAW_SHARE=1)
"$t/bin/clang" "${f[@]}" -I"$r/fw/firmware/hal" -c "$d/bench.c" -o "$d/bench.o"
"$t/bin/clang" "${f[@]}" -c "$d/start.S" -o "$d/start.o"
for a in base src;do
 "$t/bin/clang" "${f[@]}" "${defines[@]}" -I"$d/$a" -c "$d/decoder.c" -o "$d/$a.o"
 "$t/bin/ld" --gc-sections -T "$d/bench.ld" "$d/"{start,bench}.o "$d/$a.o" -o "$d/$a.elf"
 "$t/bin/objcopy" -O binary -j .text "$d/$a.elf" "$d/$a.bin"
 python3 - "$d/$a.bin" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1]); q=p.with_suffix(".padded")
q.write_bytes(bytes(0x120)+p.read_bytes())
PY
done
cd "$d"; mkdir -p work
exec 9>/tmp/dnbox-gate-firmware.lock
for a in base src; do
 flock -x 9
 FM1_STRICT=1 FM1_NO_SKIP=1 FM1_NO_PLAY=1 FM1_DUMPRAM="$d/$a.ram" "$r/restored-gate/fm1-live-ba83317a" --bin "$d/$a.padded" --entry 0x02000120 --mhz 360 --shot 370 >"$d/$a.log" 2>&1
 flock -u 9
done
python3 "$d/score.py"
