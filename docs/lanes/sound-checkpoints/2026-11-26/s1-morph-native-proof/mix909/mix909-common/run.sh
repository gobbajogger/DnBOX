#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(cd "$D/../.." && pwd); T="$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"; L="$R/s1-morph-compact/emulated-lookup"
F=(-target pi32v2 -mllvm -fast-isel=false -mcpu=r3 -mfprev1 -Os -fno-builtin -ffp-contract=off -ffunction-sections -fdata-sections -idirafter "$T/include")
"$T/bin/clang" "${F[@]}" -I"$R/fw/firmware/hal" -c "$D/bench.c" -o "$D/bench.o"
"$T/bin/clang" "${F[@]}" -c "$L/start.S" -o "$D/start.o"
"$T/bin/ld" --gc-sections -T "$L/bench.ld" "$D/"{bench,start}.o -o "$D/bench.elf"
"$T/bin/objcopy" -O binary -j .text "$D/bench.elf" "$D/bench.bin"
python - "$D" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1]);(p/'raw-padded.bin').write_bytes(bytes(0x120)+(p/'bench.bin').read_bytes())
PY
cd "$D"; mkdir -p work
exec 9>/tmp/dnbox-gate-firmware.lock; flock -x 9
FM1_STRICT=1 FM1_NO_SKIP=1 FM1_NO_PLAY=1 FM1_DUMPRAM="$D/raw.ram" "$R/restored-gate/fm1-live-ba83317a" --bin "$D/raw-padded.bin" --entry 0x02000120 --mhz 360 --shot 1000 > "$D/raw.log" 2>&1
python - "$D" <<'PY'
from pathlib import Path
import json,struct,sys
d=Path(sys.argv[1]); assert 'skipped 0 cycles' in (d/'raw.log').read_text()
i,*p=struct.unpack_from('<17I',(d/'raw.ram').read_bytes(),0x10000);assert i==0x3930394d
tags=['unity fallback','common settled','common moving without kit sends','individual-lane fallback'];r=[]
for k,tag in enumerate(tags):
 a,b,h,h2=p[4*k:4*k+4];assert h==h2 and a and b
 r.append(dict(mode=tag,baseline_ticks=a,candidate_ticks=b,saving_percent=100*(a-b)/a,hash=hex(h)))
print(json.dumps(dict(scope='PI32 native raw leaf, 512×256 audio frames per arm per case; no devices/scheduler',rows=r),indent=2))
PY
