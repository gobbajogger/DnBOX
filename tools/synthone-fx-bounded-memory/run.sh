#!/bin/sh
# Host-only Synth One FX history inventory and fidelity probe.
# Does not build firmware, link a device image, or touch the pool.
set -eu
cd "$(dirname "$0")"
ROOT=$(CDPATH= cd ../../ && pwd)
TOOL=$ROOT/tools/synthone-fx-bounded-memory
SRC=$ROOT/experiments/synthone-port-feasibility-20261114/audiokit-4.9.2/AudioKit/Core
FX=$ROOT/experiments/synthone-port-feasibility-20261114/full-reference
ENUM=$ROOT/experiments/synthone-port-feasibility-20261114/probe/portable_interface.hpp
BUILD=${TMPDIR:-/tmp}/synthone-fx-bounded-measure
mkdir -p "$BUILD"
{
  printf '%s\n' '#pragma once'
  sed -n '/^enum S1Parameter {$/,/^};$/p' "$ENUM"
} > "$BUILD/S1Parameter.h"
test -s "$BUILD/S1Parameter.h"
FLAGS="-O2 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections -DNO_LIBSNDFILE -I$BUILD -I$FX -I$FX/compat -I$SRC/Soundpipe -I$SRC/Soundpipe/lib/faust -I$SRC/Soundpipe/lib/dr_wav -I$SRC/SoundpipeExtension -I$ROOT/firmware/src/dsp"
for file in base ftbl osc pan2 phaser moogladder vdelay delay revsc buthp crossfade compressor; do
  cc -std=gnu99 $FLAGS -c "$SRC/Soundpipe/modules/$file.c" -o "$BUILD/$file.o"
done
g++ -std=c++17 $FLAGS -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-attributes \
  -o "$BUILD/measure" "$TOOL/measure.cpp" "$BUILD"/*.o -Wl,--gc-sections -lm
"$BUILD/measure" "$TOOL/MEASURED.json"
echo "wrote $TOOL/MEASURED.json"
g++ -std=c++17 $FLAGS -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-attributes \
  -o "$BUILD/ownership" "$TOOL/ownership.cpp" "$BUILD"/*.o -Wl,--gc-sections -lm
"$BUILD/ownership" "$TOOL/OWNERSHIP.json"
echo "wrote $TOOL/OWNERSHIP.json"
g++ -std=c++17 $FLAGS -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-attributes \
  -o "$BUILD/hybrid" "$TOOL/hybrid.cpp" "$BUILD"/*.o -Wl,--gc-sections -lm
"$BUILD/hybrid" "$TOOL/HYBRID.json"
echo "wrote $TOOL/HYBRID.json"
python3 "$TOOL/count_presets.py" > "$TOOL/PRESETS.json"
echo "wrote $TOOL/PRESETS.json"
