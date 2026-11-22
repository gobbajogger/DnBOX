#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Run from any directory in the firmware checkout. An optional argument
# overrides experiments/original_synth/chicago_mp4.c for both binaries.
set -eu
repo=$(git rev-parse --show-toplevel)
here=$(cd "$(dirname "$0")" && pwd)
source_file=$(realpath "${1:-$repo/experiments/original_synth/chicago_mp4.c}")
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT
for opt in 0 2; do
    set -- -std=c99 "-O$opt" -Wall -Wextra -Werror -fno-fast-math -ffp-contract=off \
        "-I$repo/experiments/original_synth" "-I$repo/tests/host" "-I$repo/firmware/src/app"
    "${CC:-cc}" "$@" "$here/edge.c" "$source_file" -lm -o "$out/edge"
    "$out/edge"
    "${CC:-cc}" "$@" "-DCHICAGO_SOURCE=\"$source_file\"" "$here/wave.c" -lm -o "$out/wave"
    "$out/wave"
done
