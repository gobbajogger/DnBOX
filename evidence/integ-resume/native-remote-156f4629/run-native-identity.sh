#!/bin/sh
set -eu
out=build/host/dnr2-scratch
mkdir -p "$out"
[ -f build/gen/x0x_drum_tables.h ] || sh host/build_host.sh --generate-only >/dev/null
. tests/host/original_build.sh
sources=$(printf '%s\n' "$ORIGINAL_SOURCES" | sed 's@firmware/src/app/engine.c@@')
# shellcheck disable=SC2086
"${CC:-cc}" -O1 -ffp-contract=off -std=c99 -DX0X_HOST -D_POSIX_C_SOURCE=200809L \
    -DX0X_DNR2_SCRATCH=1 -DDNR2_NATIVE_HOST_FIXTURE=1 -DX0X_TANH_BOOT_TABLE=1 \
    -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen \
    tests/host/dnr2_scalar_engine_test.c tests/host/dnr2_scalar_engine_fixture.c \
    $ORIGINAL_FLAGS $sources -lm -o "$out/engine-native"
"$out/engine-native"
