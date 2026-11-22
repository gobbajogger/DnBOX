#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# C dispatcher and real host engine proof. Optional pinned Nightly checkout
# exercises the shipping client module via its non-browser DOM fixture.
set -eu
cd "$(dirname "$0")/../.."
if [ "$#" -gt 1 ]; then
    echo "usage: $0 [pinned-nightly-checkout]" >&2
    exit 2
fi
out=build/host/dnr2-scalar
mkdir -p "$out"
if [ ! -f build/gen/x0x_drum_tables.h ]; then
    sh host/build_host.sh --generate-only >/dev/null
fi
compile_dispatch()
{
    "${CC:-cc}" -std=c99 -O2 -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-variable \
        -Ibuild/gen "$@" tests/host/dnr2_scalar_dispatch_test.c
}
compile_dispatch -o "$out/stock"
if "$out/stock" > "$out/stock.log" 2>&1; then
    echo 'Stock unexpectedly advertises the test scalar' >&2
    exit 1
else
    test "$?" -eq 1
    cat "$out/stock.log"
fi
compile_dispatch -DDNR2_TEST_CANDIDATE -o "$out/dispatch"
"$out/dispatch"
sh tests/host/run_project_protocol.sh

. tests/host/original_build.sh
# Pull the existing engine TU through the test fixture to observe the private
# ring and gain. Keep the same flags and every other source in the stock plan.
sources=$(printf '%s\n' "$ORIGINAL_SOURCES" | sed 's@firmware/src/app/engine.c@@')
# shellcheck disable=SC2086
"${CC:-cc}" -O1 -ffp-contract=off -std=c99 -DX0X_HOST -D_POSIX_C_SOURCE=200809L \
    -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen tests/host/dnr2_scalar_engine_test.c \
    tests/host/dnr2_scalar_engine_fixture.c $ORIGINAL_FLAGS $sources -lm -o "$out/engine"
"$out/engine"
if [ "$#" -eq 1 ]; then
    node tests/host/dnr2_scalar_browser_client_test.mjs "$out/dispatch" "$1"
fi
