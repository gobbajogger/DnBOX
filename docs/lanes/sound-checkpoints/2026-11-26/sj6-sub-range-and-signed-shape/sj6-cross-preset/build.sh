#!/bin/bash
set -euo pipefail
D=$(realpath "$(dirname "$0")")
W=$(dirname "$D")
T=$(realpath "${1:-$W/sj6-hp-exact-exp-fit/kept-elite-1}")
O=$(realpath "${2:-$D/run.private}")
F=$(tr ' ' '\n' < "$D/flags-source.txt"|grep -E '^-D(X0X_(SJ6|SUPERJ6|S1_SCALE|S1_BEND_RANGE)|DNBOX_TYPED_CONTROLS)'|grep -vE 'ZERO_LEAF|CUTOFF_LATCH|SCALE_NATIVE|BEND_TYPED|BEND_RANGE|DNBOX_TYPED|NATIVE_RANDOM2|RANDOM5_6|SEQ_DEST_NATIVE|ARP_ADAPTER_NATIVE|VELOCITY_SETTINGS')
more=(${SJ6_EXTRA_FLAGS:-})
if [[ -n "${SJ6_RUNTIME_OVERRIDE:-}" ]]; then
 more+=("-DSJ6_RUNTIME_OVERRIDE=\"$SJ6_RUNTIME_OVERRIDE\"")
fi
cd "$T"
sha256sum firmware/src/{app/original_runtime.c,dsp/juno6.c,dsp/juno6_factory.inc,dsp/dnbx_core.c}
cc -std=c99 -O2 -ffp-contract=off -ffunction-sections -fdata-sections -Wno-attributes -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_ORIGINAL_SYNTH=1 -DX0X_ARP_HOOKS=1 "${more[@]}" $F -Ifirmware/src/app -Ifirmware/src/dsp -Ifirmware/src -Iexperiments/original_synth "${SJ6_RENDER_SOURCE:-$D/render.c}" "$D/foreign-fail-closed.c" firmware/src/dsp/juno6.c "${SJ6_DNBX_CORE_OVERRIDE:-firmware/src/dsp/dnbx_core.c}" -Wl,--gc-sections -lm -o "$O"
