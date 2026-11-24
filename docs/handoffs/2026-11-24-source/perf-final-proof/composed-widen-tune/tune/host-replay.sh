#!/bin/sh
set -eu
cd "${TUNING_SOURCE:-.}"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_ARP_HOOKS=1 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS="-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 ${HOST_EXTRA:--Wl,--wrap=juno6_init}"
export SAN_FLAGS="${SAN_FLAGS:--fsanitize=shift -fno-sanitize-recover=shift}"
export SMC_UI_TEST_FILE="${SMC_UI_TEST_FILE:-tests/host/ui_superj6_layer1_tuning_test.c}"
export SMC_UI_OUT="${SMC_UI_OUT:-/tmp/sj6-id72-host}"
sh tests/host/sj6-smc-ui.sh
