#!/usr/bin/env bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D")
P=${1:?directory containing pinned Mac chain cases}
B="${2:-$D/build-elite-v2}";T="$B/firmware/src";S=$(mktemp -d)
trap 'rm -r "$S"' EXIT
python3 - "$T/app/engine.c" > "$S/engine-owner.inc" <<'PY'
import sys
s=open(sys.argv[1]).read();a=s.index('static sj6_zero_fx_channel original_sj6_zero_leaf')
print(s[a:s.index('#endif',a)])
PY
F=(-DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NATIVE_ROOM=1 -DX0X_SJ6_AUX_LFO_NATIVE=1 -DX0X_SJ6_NATIVE_UNISON=1 -DX0X_SJ6_EXACT_ZERO_LEAF=1)
cc -std=c99 -O2 -ffp-contract=off -ffunction-sections -fdata-sections -Wno-attributes "${F[@]}" -I"$S" -I"$T/dsp" -I"$T/app" -I"$T" -I"$B/experiments/original_synth" "$D/connected.c" "$T/dsp/juno6.c" "$T/dsp/dnbx_core.c" -Wl,--gc-sections -Wl,--version-script=<(printf '{ global: route_begin; route_span; local: *; };') -Wl,--no-undefined -fPIC -shared -lm -o "$S/route.so"
python3 "$D/score-connected.py" "$S/route.so" "$P"
