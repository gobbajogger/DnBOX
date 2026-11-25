#!/bin/sh
set -eu
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1 SAN_FLAGS='-fsanitize=undefined -fno-sanitize-recover=all'
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -DDNBOX_137_HOST_WITNESS -DX0X_SJ6_FILTER_VEL_PROBE=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_AUTOPAN_PROBE=1 -DS1C_FAST=1 -Wl,--wrap=s1_part_init -Wl,--wrap=s1u_autopan_authored -Iout out/osc.o out/pan2.o'
SMC_UI_OUT="$PWD/out" SMC_UI_TEST_FILE=tests/host/s1_autopan_dsp.c sh tests/host/sj6-smc-ui.sh
