#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(cd "$D/../.." && pwd); T="$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"; L="$R/s1-morph-compact/emulated-lookup"
A="$R/s1-morph-native-sched/tree/experiments/original_synth"; B="$R/s1-morph-native-chicago/tree/experiments/original_synth"
F=(-target pi32v2 -mllvm -fast-isel=false -mcpu=r3 -mfprev1 -O2 -fno-builtin -ffp-contract=off -ffunction-sections -fdata-sections -idirafter "$T/include")
"$T/bin/clang" "${F[@]}" -I"$R/fw/firmware/hal" -I"$A" -I"$R/fw/firmware/src/app" -c "$D/bench.c" -o "$D/bench.o"
"$T/bin/clang" "${F[@]}" -c "$L/start.S" -o "$D/start.o"
for n in baseline candidate;do
 X=$A; [[ $n == baseline ]] || X=$B
 "$T/bin/clang" "${F[@]}" -I"$X" -I"$R/fw/firmware/src/app" -c "$X/chicago_mp4.c" -o "$D/$n-mp.o"
 "$T/bin/ld" --gc-sections -T "$L/bench.ld" "$D/"{bench,start}.o "$D/$n-mp.o" -o "$D/$n.elf"
 "$T/bin/objcopy" -O binary -j .text "$D/$n.elf" "$D/$n.bin"
 python - "$D/$n" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1]);Path(str(p)+'-padded.bin').write_bytes(bytes(0x120)+p.with_suffix('.bin').read_bytes())
PY
done
cd "$D"; mkdir -p work
exec 9>/tmp/dnbox-gate-firmware.lock; flock -x 9
for n in baseline candidate;do
 FM1_STRICT=1 FM1_NO_SKIP=1 FM1_NO_PLAY=1 FM1_DUMPRAM="$D/$n.ram" "$R/restored-gate/fm1-live-ba83317a" --bin "$D/$n-padded.bin" --entry 0x02000120 --mhz 360 --shot 500 > "$D/$n.log" 2>&1
done
python - "$D" <<'PY'
from pathlib import Path
import json,struct,sys
d=Path(sys.argv[1])
def get(k):
 assert 'skipped 0 cycles' in (d/(k+'.log')).read_text()
 x=struct.unpack_from('<10I',(d/(k+'.ram')).read_bytes(),0x10000)
 assert x[0]==0x43394d50 and not x[9]
 return x
a,b=get('baseline'),get('candidate')
assert a[5:]==b[5:]
print(json.dumps({'scope':'Raw PI32v2 same kernel; 16,384 frames per case; no audio scheduler',
 'rows':[dict(mode=m,baseline_ticks=x,candidate_ticks=y,saving_pct=100*(x-y)/x,output_hash=hex(h)) for m,x,y,h in zip(('held, no xmod','held, xmod','ramps, no xmod','ramps, xmod'),a[1:5],b[1:5],a[5:9])]},indent=2))
PY
