#!/usr/bin/env bash
set -euo pipefail
D=$PWD/sj6-j60-latch R=$PWD/sj6-raw-j60-right W=$PWD/sj6-raw-j60-wet B=$PWD/sj6-cross-preset T=$PWD/sj6-hp-exact-exp-fit/kept-elite-1
export TMPDIR="$D"; O=$(mktemp -d); trap 'rm -rf "$O"' EXIT
F=$(tr " " "\n" < "$B/flags-source.txt"|grep -E "^-DX0X_(SJ6|SUPERJ6)" | grep -vE "ZERO_LEAF|CUTOFF_LATCH|SCALE_NATIVE|BEND_TYPED|BEND_RANGE|DNBOX_TYPED|NATIVE_RANDOM2|RANDOM5_6|SEQ_DEST_NATIVE|ARP_ADAPTER_NATIVE|VELOCITY_SETTINGS")
for mode in stock right latch; do
  python3 "$W/wet.py" "$T/firmware/src/dsp/juno6_j60_native.inc" "$O/wet.inc" "$([[ "$mode" != stock ]] && echo post || echo "$mode")"
  if [[ "$mode" != stock ]]; then python3 "$R/right.py" "$O/wet.inc"; fi
  if [[ "$mode" == latch ]]; then python3 "$D/latch.py" "$O/wet.inc"; fi
  cc -shared -fPIC -fvisibility=hidden -O2 -ffp-contract=off -ffunction-sections -fdata-sections -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 $F -I"$T/firmware/src/dsp" -I"$T/firmware/src" -I"$T/experiments/original_synth" -I"$O" "$D/live.c" -Wl,--gc-sections -lm -o "$O/$mode.so"
done

python3 - "$O" <<'PASS'
import ctypes as ct,sys
from pathlib import Path
for mode in ('stock','right','latch'):
 lib=ct.CDLL(str(Path(sys.argv[1])/f'{mode}.so'))
 got=lib.live_guard(int(mode=='latch'))
 assert got>0,(mode,got)
 print(mode,got,"checks with nonzero prior-state distinction; inactive identity/reset exact")
PASS
