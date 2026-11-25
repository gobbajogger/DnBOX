#!/bin/sh
set -eu
W=${1:-$PWD}
D="$W/perf-final-proof/s1-init-mod-wheel"
T=$(cat "$D/checkout")
source=${2:-$T/source}
out=${3:-$T/out}
mkdir -p "$out"
awk '/^static int smk_pitch_parameter\(/ {p=1} p {print} p && /^}/ {exit}' "$source/firmware/src/app/main_fm1.c" >"$out/mod-wheels.inc"
awk '/^static void smk_wheels\(void\)$/ {p=1} p {print} p && /^}/ {exit}' "$source/firmware/src/app/main_fm1.c" >>"$out/mod-wheels.inc"
test -s "$out/mod-wheels.inc"
TUNING_SOURCE="$source" SMC_UI_TEST_FILE="$D/ui_s1_init_mod_wheel_test.c" SMC_UI_OUT="$out" \
HOST_EXTRA="-I$source/tests/host -I$out -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -Wl,--wrap=s1_part_init ${TEST_FLAGS:-}" \
    sh "$W/perf-final-proof/sj6-picker-host-hook/replay-host.sh"
