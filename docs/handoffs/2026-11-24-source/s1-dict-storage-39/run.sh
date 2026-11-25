#!/bin/sh
set -eu
export X0X_SUPERJ6_FACTORY=1 X0X_SYNTHONE_PATCH=1 X0X_S1C=1
export SAN_FLAGS='-fsanitize=undefined -fno-sanitize-recover=all'
flags='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DDNBOX_137_HOST_WITNESS=1 -DX0X_S1_FEEDBACK_PROBE=1'
case "$1" in
now) flags="$flags -DX0X_S1_DELAY_TRACK_TEST=1 -Wl,--wrap=s1_part_init";;
prior) flags="$flags -DDICT_PREVIOUS=1";;
trial) flags="${flags% -DX0X_S1_FEEDBACK_PROBE=1} -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -DX0X_S1_DELAY_TRACK_TEST=1 -DDICT_TRIAL=1 -Wl,--wrap=s1_part_init";;
*) exit 2;;
esac
export X0X_HOST_DEFS="$flags"
SMC_UI_TEST_FILE=tests/host/dict_fixture.c SMC_UI_OUT="$PWD/out" sh tests/host/sj6-smc-ui.sh
