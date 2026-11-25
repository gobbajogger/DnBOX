#!/bin/sh
set -eu
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DX0X_SJ6_ALT3_WITNESS=1 -Itests/host -Wl,--wrap=s1_part_init -Wl,--wrap=juno6_init -Wl,--wrap=juno6_alt3_process'
export SMC_UI_TEST_FILE=tests/host/sj6-mod-wheel-host.c SMC_UI_OUT=out
exec sh tests/host/sj6-smc-ui.sh
