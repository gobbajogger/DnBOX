#!/bin/sh
# Compile the public runtime through the same source list as the Juno runtime test.
set -eu
tree=$(realpath "$1")
out=$(realpath "$2")
driver=$(realpath "$(dirname "$0")/render_public.c")
cd "$tree"
. tests/host/original_build.sh
sources=$(python3 -c "import tools.build as b; print(' '.join(b.ORIGINAL_SYNTH_UNITS+b.ORIGINAL_SYNTH_BACKENDS))")
# ORIGINAL_FLAGS and sources are the repository's space-separated compiler argument lists.
${CC:-cc} -std=c99 -O2 -ffunction-sections -fdata-sections -DDNBX_DIAGNOSTIC_COUNTERS=0 \
    -DX0X_HOST $ORIGINAL_FLAGS -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
    "$driver" $sources -Wl,--gc-sections -lm -o "$out/renderer"
"$out/renderer" "$out"
