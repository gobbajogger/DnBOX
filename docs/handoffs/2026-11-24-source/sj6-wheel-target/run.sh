#!/bin/bash
set -euo pipefail
d=$(realpath "$(dirname "$0")")
mode=${1:-green}
base=${DNBOX_EXACT_BASE:-"$d/../controls-init-only/source-ui-pitch-wheel"}
w=$(mktemp -d "$d/host-build.XXXXXX")
trap 'rm -rf -- "$w"' EXIT
cp -a "$base"/{assets,ble,build,experiments,firmware,host,tests,tools} "$w/"
cp "$d/wheel-fixture.c" "$w/tests/host/"
cd "$w"
if [[ $mode != red ]]; then patch -s -p1 --fuzz=0 < "$d/wheel-target.patch"; fi
sed -n '/^static int smk_pitch_parameter(int v)$/,/^}$/p; /^static void smk_wheels(void)$/,/^}$/p' firmware/src/app/main_fm1.c > tests/host/wheels.inc
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -Itests/host -Wl,--wrap=s1_part_init -Wl,--wrap=juno6_init'
export SMC_UI_TEST_FILE=tests/host/wheel-fixture.c
k=${1:-green}
SMC_UI_OUT="$w/host-out" sh tests/host/sj6-smc-ui.sh > "$d/$k.log" 2>&1
tail -3 "$d/$k.log"
