#!/bin/sh
set -eu
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS='-pipe -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -Itests/host -Wl,--wrap=juno6_init -Wl,--wrap=original_runtime_render'
export SAN_FLAGS='-fsanitize=undefined -fno-sanitize-recover=all'
SMC_UI_TEST_FILE=tests/host/ui_native_gate.c SMC_UI_OUT=out sh tests/host/sj6-smc-ui.sh
