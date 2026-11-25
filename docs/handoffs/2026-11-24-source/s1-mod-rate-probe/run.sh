#!/bin/sh
set -eu
W=$(cd "${1:-$PWD}" && pwd)
D="$W/perf-final-proof/s1-mod-rate-probe"
S="${2:-$W/perf-final-proof/controls-init-only/source-mod-wheels}"
T=$(mktemp -d "$D/run.XXXXXX")
trap 'rm -rf "$T"' EXIT
mkdir -p "$T/out"
cp -a "$S" "$T/source"
patch -d "$T/source" -p1 --fuzz=0 < "$D/cc1-rate-probe.patch"
out="$T/out"
source="$T/source"
awk '/^static int smk_pitch_parameter\(/ {p=1} p {print} p && /^}/ {exit}' "$source/firmware/src/app/main_fm1.c" >"$out/mod-wheels.inc"
awk '/^static void smk_wheels\(void\)$/ {p=1} p {print} p && /^}/ {exit}' "$source/firmware/src/app/main_fm1.c" >>"$out/mod-wheels.inc"
TUNING_SOURCE="$source" SMC_UI_TEST_FILE="$D/wheel-rate-host.c" SMC_UI_OUT="$out" \
HOST_EXTRA="-I$source/tests/host -I$out -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DX0X_S1_MOD_RATE_PROBE=1 -Wl,--wrap=s1_part_init ${TEST_FLAGS:-}" \
    sh "$W/perf-final-proof/sj6-picker-host-hook/replay-host.sh"
