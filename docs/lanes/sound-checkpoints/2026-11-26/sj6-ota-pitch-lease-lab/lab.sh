#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/../.."; R=$PWD; X="$R/sj6-ota-history/pitchenv-lease"; W=/tmp/phaser-warm-own.Y0XNbQ/tree/firmware/src/dsp
O=$(mktemp -d "$X/tmp.XXXXXX");trap 'rm -rf "$O"' EXIT;export TMPDIR="$O"
mkdir -p "$O/firmware/src/dsp"; cp "$W/dnbx_core.h" "$O/firmware/src/dsp";
(cd "$O"; patch --fuzz=0 -p1 -s < "$X/pitch-layout.patch")
python3 sj6-ota-history/transposed-auth/upstream.py sj6-raw-main-ota/physical.c "$O/fused.c"
sed -E 's/fmaf\(-a,mid,x\)/x-a*mid/;s/fmaf\(-b,y,mid\)/mid-b*y/' "$O/fused.c" > "$O/separate.c"
python3 sj6-ota-history/hoist/wire.py "$O/separate.c" "$O/hoisted.c"
{ printf '#include "dnbx_core.h"\n#include <math.h>\n#define fmaf(a,b,c) (((a)*(b))+(c))\n'; cat "$O/hoisted.c" "$X/lab.inc" "$X/lab-test.inc"; } > "$O/lab.c"
FLAGS=$(cat all-controls-remote-join/compile-flags.txt)
cc -Os -ffp-contract=off -I"$O/firmware/src/dsp" -I"$W" $FLAGS -DDNBX_DIAGNOSTIC_SJ6_PITCH_LEASE=1 "$O/lab.c" -lm -o "$O/lab"
"$O/lab"
