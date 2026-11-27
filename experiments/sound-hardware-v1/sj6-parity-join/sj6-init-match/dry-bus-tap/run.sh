#!/usr/bin/env bash
set -euo pipefail
here=$(cd "$(dirname "$0")"; pwd)
src=$(cd "$1"; pwd)
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
cp "$here"/{build.sh,render.c} "$here/../native-init-ab/cue.h" "$T/"
cp "$src/tree/firmware/src/app/original_runtime.c" "$T/"
patch -s "$T/original_runtime.c" "$here/stage-tap.patch"
bash "$T/build.sh" "$src" >&2
(
  cd "$T"
  ./render 32 3 0 >terminal.f32
  cmp terminal.f32 pre-effects.f32
  sha256sum pre-effects.f32 terminal.f32
  wc -c terminal.f32
)
