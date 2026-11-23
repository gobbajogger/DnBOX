#!/bin/sh
# SuperJ6 row 5: all 16 editor-page controls and the chorus change the output; bypass is bit-exact.
# Built twice: the stock runtime and the X0X_SUPERJ6_FACTORY runtime.
set -eu
cd "$(dirname "$0")/../.."
OUT=${1:-build/host/superj6-controls}; mkdir -p "$OUT"
for factory in 0 1; do
    X0X_SUPERJ6_FACTORY=$factory; export X0X_SUPERJ6_FACTORY
    . tests/host/original_build.sh
    SOURCES=$(python3 -c "import tools.build as b; print(' '.join(b.ORIGINAL_SYNTH_UNITS+b.ORIGINAL_SYNTH_BACKENDS))")
    ${CC:-cc} -std=c99 -O2 -ffunction-sections -fdata-sections -DX0X_HOST $ORIGINAL_FLAGS ${SAN_FLAGS:-} -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen tests/host/superj6_controls_test.c $SOURCES -Wl,--gc-sections -lm -o "$OUT/test$factory"
    echo "factory=$factory"; "$OUT/test$factory"
done
