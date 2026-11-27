#!/usr/bin/env bash
set -euo pipefail
h=$(cd "$(dirname "$0")";pwd)
src=$(cd "$1";pwd)
reference=$(realpath "$2")
baseline=$(realpath "$3")
t=$(mktemp -d)
trap 'rm -rf "$t"' EXIT
cp "$h"/{render.c,build.sh} "$h/../native-init-ab/cue.h" "$t/"
for pair in dsp/dnbx_core.c app/original_runtime.c; do
  n=${pair##*/}
  cp "$src/tree/firmware/src/$pair" "$t/$n"
  patch -s "$t/$n" "$h/$n.patch"
done
bash "$t/build.sh" "$src" >&2
(cd "$t"
 for m in 0 3; do
   ./render 32 3 "$m" >"terminal$m.f32"
   mv mix.f32 "mix$m.f32"
 done
)
python "$h/score.py" "$t" "$reference" "$baseline"
