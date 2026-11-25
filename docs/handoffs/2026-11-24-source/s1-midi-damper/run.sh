#!/bin/bash
set -euo pipefail
d=$(realpath "$(dirname "$0")")
base=${DNBOX_EXACT_BASE:-"$d/../controls-init-only/source-all-wheel"}
mode=${1:-green}
w=$(mktemp -d "$d/tmp.XXXXXX")
trap 'rm -rf -- "$w"' EXIT
cp -a "$base"/{assets,ble,build,experiments,firmware,host,tests,tools} "$w/"
cp "$d/test.c" "$w/tests/host/s1-midi-damper.c"
cd "$w"
if [[ $mode == green ]]; then patch -s -p1 --fuzz=0 < "$d/cc64.patch"; fi
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -Itests/host -Wl,--wrap=s1_part_init'
export SMC_UI_TEST_FILE=tests/host/s1-midi-damper.c
SMC_UI_OUT="$w/bin" sh tests/host/sj6-smc-ui.sh > "$d/$mode.log" 2>&1
tail -3 "$d/$mode.log"
