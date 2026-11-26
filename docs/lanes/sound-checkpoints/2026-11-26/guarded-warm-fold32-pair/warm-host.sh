#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
N=$(cat warm-workpath)
D="$N/tree/firmware/src/dsp"
for x in cold warm; do
 if [ "$x" = cold ]; then src="$N/cold.c"; else src="$D/dnbx_core.c"; fi
 python3 - "$src" "$N/$x-tap.c" <<'PY'
from pathlib import Path
import sys
a,b=map(Path,sys.argv[1:]);s=a.read_text()
if not s.startswith('#define SJ6_WAVE_PROBE 1'):raise SystemExit('inactive')
m='                if (osc_gain[l]!=1.f) x*=osc_gain[l];'
assert s.count(m)==1
b.write_text(s.replace(m,m+'\n                extern float carrier_tap[2][128], raw_tap[2][128];\n                carrier_tap[l][k]+=velocity*amp*x;raw_tap[l][k]+=x;'))
PY
 f=(-O2 -std=c99 -ffp-contract=off -Wno-attributes -DX0X_HOST=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NOISE_SUB_NATIVE=1 -DX0X_SJ6_OTA_NATIVE=1 -DX0X_SJ6_RANDOM5_6_NATIVE=1 -DX0X_SJ6_SEQ_DEST_NATIVE=1 -I"$D")
 cc "${f[@]}" "$N/$x-tap.c" phase-src/live/dynamic.c -DEXPECT_KEEP_CLOCK=$([ "$x" = warm ] && echo 1 || echo 0) -lm -o "$N/$x-live"
 "$N/$x-live" > "warm-$x-dynamic.log"
 cc "${f[@]}" "$N/$x-tap.c" phase-src/live/clock_switch.c -lm -o "$N/$x-switch"
 "$N/$x-switch" > "warm-$x-switch.log"
done
diff -u <(cut -d' ' -f1-5 phase-src/live/clock-central.txt) <(cut -d' ' -f1-5 warm-warm-switch.log)
echo 'Exact four physical source switch phase endpoints against known original-backed fixture'
cat warm-{cold,warm}-dynamic.log
