#!/usr/bin/env bash
set -euo pipefail
SRC=${1:?stock source scratch tree}; OUT=${2:?new project file}
cd "$SRC"
sh host/build_host.sh --generate-only
. tests/host/original_build.sh
f=experiments/sound-hardware-v1/selected-native/project-solo.c
sed 's/const int notes = kit < 0 || kit == 2;/const int notes = 0;/' experiments/sound-hardware-v1/selected-native/project.c > "$f"
bin=$(mktemp)
trap 'rm -f -- "$f" "$bin"' EXIT
cc -std=c11 -O1 -DX0X_HOST -D_POSIX_C_SOURCE=200809L -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen $ORIGINAL_FLAGS "$f" $ORIGINAL_SOURCES -lm -o "$bin"
"$bin" 2 0 -1 "$OUT"
