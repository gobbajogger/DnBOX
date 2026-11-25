#!/bin/bash
set -euo pipefail
d=$(realpath "$(dirname "$0")")
cd "$d/source"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -Itests/host -Wl,--wrap=s1_part_note'
export SMC_UI_TEST_FILE=tests/host/ui_s1_arp_signed_octave_test.c
if [[ ${1:-} == existing ]]; then export SMC_UI_TEST_FILE=tests/host/ui_arp_keyboard_test.c; export X0X_HOST_DEFS=${X0X_HOST_DEFS% -Wl,--wrap=s1_part_note}; fi
k=${1:-target}
SMC_UI_OUT="$d/host-$k" sh tests/host/sj6-smc-ui.sh > "$d/$k.log" 2>&1
tail -4 "$d/$k.log"
