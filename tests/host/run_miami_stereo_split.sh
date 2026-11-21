#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
root=$(cd "$(dirname "$0")/../.." && pwd)
out="$root/build/host/miami-stereo"
mkdir -p "$out"
if [ "$#" -eq 2 ] && [ "$1" = --legacy ]; then
    # Use the archived WBL1 implementation and matching test and headers
    # together (0c228a97e67a73c0b377f62728603d94fb1bec66).
    tree=$2
    test -f "$tree/experiments/wobble/wobble.c" || {
        echo "Missing historical WBL1 source in $tree" >&2
        exit 2
    }
    ${CC:-cc} -std=c99 -O2 -Wall -Wextra -Werror -Wno-error=attributes -ffp-contract=off \
        -DX0X_SYNTH_HOST_TEST -I"$tree/firmware/src" -I"$tree/experiments/wobble" \
        "$tree/tests/host/miami_stereo_split_test.c" "$tree/experiments/wobble/wobble.c" \
        -lm -o "$out/legacy"
    exec "$out/legacy"
fi
if [ "$#" -ne 0 ]; then
    echo "usage: $0 [--legacy WBL1_SOURCE_TREE]" >&2
    exit 2
fi
cd "$root"
. tests/host/original_build.sh
case "$(uname -s)" in
    Darwin) gc_sections=-Wl,-dead_strip ;;
    *) gc_sections=-Wl,--gc-sections ;;
esac
sources=$(python3 -c 'import tools.build as b; print(" ".join(b.ORIGINAL_SYNTH_UNITS + b.ORIGINAL_SYNTH_BACKENDS))')
${CC:-cc} -std=c99 -O2 -ffunction-sections -fdata-sections -DX0X_HOST $ORIGINAL_FLAGS \
    -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
    tests/host/miami_stereo_test.c $sources "$gc_sections" -lm -o "$out/production"
exec "$out/production"
