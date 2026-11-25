#!/bin/sh
set -eu
stage=${STAGE:?prior or after}
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_ARP_HOOKS=1 PYTHONDONTWRITEBYTECODE=1
export SMC_UI_TEST_FILE="tests/host/join-page24-census.c" SMC_UI_OUT="$PWD/out/page24"
export SAN_FLAGS="-fsanitize=undefined -fno-sanitize-recover=all"
flags="-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -DDNBOX_137_HOST_WITNESS -DX0X_SJ6_FILTER_VEL_PROBE=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_SJ6_BEND_TYPED_PROBE=1 -DX0X_SJ6_ALT3_WITNESS=1 -DX0X_SJ6_MOD_TYPED_PROBE=1 -DX0X_S1_FENCED_CLOCK=1 -DX0X_SJ6_ZERO_GATE_PROBE=1 -DX0X_SJ6_GATE_DEADLINE_PROBE=1 -DX0X_S1_CRUSH_LFO_PROBE=1 -DX0X_SJ6_DELAY_CAP=11026 -DX0X_SJ6_DELAY_WITNESS=1 -DX0X_S1_AUTOPAN_PROBE=1 -DX0X_S1_FEEDBACK_PROBE=1 -DX0X_SJ6_CLASSIC_I_PROBE=1 -DX0X_SJ6_CLASSIC_SWITCH_WITNESS=1 -DCOMPACT_PAGE_BUDGET=1 -I$PWD/tests/host"
if [ "$stage" = after ]; then flags="$flags -DDNBX_PAGE24=1";fi
X0X_HOST_DEFS="$flags" sh tests/host/sj6-smc-ui.sh >"out/page24/$stage.log" 2>"out/page24/$stage.err" || { grep -E 'error:|FAIL|runtime error' "out/page24/$stage.err" | head; exit 1; }
grep -E '^(frame|page |list |global )' "out/page24/$stage.log" >"out/page24/$stage.census"
grep -E '^(page-bytes=|checks=)' "out/page24/$stage.err"
