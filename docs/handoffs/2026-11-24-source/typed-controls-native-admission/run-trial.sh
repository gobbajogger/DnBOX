#!/bin/sh
set -eu
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1 SAN_FLAGS='-fsanitize=undefined -fno-sanitize-recover=all'
flags='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_AUTOPAN_PROBE=1'
case "$1" in
old) flags="${flags%% -DX0X_S1_MOD_RATE_PROBE=*} -DTRIAL_OLD=1";;
writer) flags="$flags -DDNBOX_137_HOST_WITNESS=1 -DTRIAL_WRITER=1";;
trial) flags="$flags -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1";;
*) exit 2;;
esac
export X0X_HOST_DEFS="$flags"
export S1_FILTER_PROJECTS="$TRIAL_INPUT"
SMC_UI_OUT="$PWD/out" SMC_UI_TEST_FILE=tests/host/trial_fixture.c sh tests/host/sj6-smc-ui.sh
