#!/bin/bash
set -euo pipefail
D=$(realpath "$(dirname "$0")")
W=$(dirname "$D"); B="$W/sj6-cross-preset"; T="$W/sj6-hp-exact-exp-fit/kept-elite-1"; C="$T/firmware/src/dsp/dnbx_core.c"
export OPENBLAS_NUM_THREADS=1
O=$(mktemp -d "$W/sj6-mix-run.XXXXX"); trap 'rm -r "$O"' EXIT
python "$B/meter.py" "$T/firmware/src/app/original_runtime.c" "$O/runtime.c"
F=$(tr ' ' '\n' < "$B/flags-source.txt"|grep -E '^-D(X0X_(SJ6|SUPERJ6|S1_SCALE|S1_BEND_RANGE)|DNBOX_TYPED_CONTROLS)'|grep -vE 'ZERO_LEAF|CUTOFF_LATCH|SCALE_NATIVE|BEND_TYPED|BEND_RANGE|DNBOX_TYPED|NATIVE_RANDOM2|RANDOM5_6|SEQ_DEST_NATIVE|ARP_ADAPTER_NATIVE|VELOCITY_SETTINGS')
sha256sum "$C" "$B/render.c" "$B/flags-source.txt" "$B"/projected-v2.private/?-dry.private.txt > "$D/pins.txt"
for a in base divisor center;do
 cd "$T"
 python "$D/variant.py" "$C" "$O/mixer.c" "$a"
 cc -std=c99 -O2 -ffp-contract=off -ffunction-sections -fdata-sections -Wno-attributes -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_ORIGINAL_SYNTH=1 -DX0X_ARP_HOOKS=1 "-DSJ6_RUNTIME_OVERRIDE=\"$O/runtime.c\"" $F -Ifirmware/src/app -Ifirmware/src/dsp -Ifirmware/src -Iexperiments/original_synth "$B/render.c" "$B/foreign-fail-closed.c" firmware/src/dsp/juno6.c "$O/mixer.c" -Wl,--gc-sections -lm -o "$O/run"
 for i in 0 1 2;do
  "$O/run" "$B/projected-v2.private/$i-dry.private.txt" "$O/$a-$i.raw" 2>"$O/$a-$i.txt"
 done
done
cd "$W"
python "$D/score.py" "$O" "$B" "$T/experiments/sound-hardware-v1/sj6-factory"
