#!/bin/sh
set -eu
repo=$(cd "$1" && pwd)
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
base="$repo/firmware/src/dsp/synthone/experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe"
oracle="$repo/tests/fixtures/synthone-factory/fullfx/upstream/AudioKit/Core/Soundpipe/modules/vdelay.c"
cc -std=gnu99 -O2 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections -DNO_LIBSNDFILE \
   -I"$base" -I"$base/lib/dr_wav" "$here/independent_histories.c" "$oracle" "$base/modules/base.c" -Wl,--gc-sections -lm -o "$tmp/witness"
"$tmp/witness"
