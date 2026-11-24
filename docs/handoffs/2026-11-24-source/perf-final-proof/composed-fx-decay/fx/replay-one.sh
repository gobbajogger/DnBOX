#!/bin/bash
set -eu
cd "$1"; name=$2; fixture=$3
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
SMC_UI_OUT="$PWD/$name" SMC_UI_TEST_FILE="tests/host/$fixture.c" sh tests/host/sj6-smc-ui.sh > "$name.log" 2>&1
