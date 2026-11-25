#!/bin/bash
set -euo pipefail
H=$(cd "$(dirname "$0")" && pwd)
R=$(dirname "$H")
T="$H/tree/firmware/src/dsp/synthone/sequencer"
flags=($(grep -oE -- '-D(X0X_SJ6_(NATIVE_ZERO_GATE|NATIVE_POSITIVE_GATE|SEQ_SETTINGS_NATIVE|SEQ_DEST_NATIVE|NATIVE_RANDOM2|RANDOM5_6_NATIVE)|X0X_S1_(FENCED_CLOCK|CLOCK_NATIVE|SCALE_NATIVE)|X0X_SUPERJ6_BUILTIN_INIT|DNBOX_TYPED_CONTROLS_NATIVE_TEST)=[01]' "$R/all-controls-remote-join/compile-flags.txt" | sort -u))
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
"${CC:-cc}" -O2 -I"$T" "${flags[@]}" "$H/oracle.c" "$T/S1Sequencer.c" -o "$W/oracle"
"$W/oracle"
