#!/usr/bin/env bash
set -euo pipefail
here=$(cd "$(dirname "$0")"; pwd)
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
cp "$here/render.c" "$here/../native-init-ab"/{build.sh,cue.h} "$T/"
bash "$T/build.sh" "$1" >&2
for m in 0 1 2 3; do
  "$T/render" 32 3 "$m" >"$T/defaults-$m.f32" 2>"$T/defaults-$m.log"
done
python "$here/score.py" "$2" "$T"
