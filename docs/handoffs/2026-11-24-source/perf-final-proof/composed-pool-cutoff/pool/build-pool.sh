#!/bin/bash
set -eu
cd "$1"; arm=$2; test=$3; variant=$4; name=$5
if [[ $variant == on ]]; then
 export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1
 flags='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
else
 export X0X_SUPERJ6_FACTORY=0 X0X_S1C=0 X0X_SYNTHONE_PATCH=0
 flags='-DX0X_SUPERJ6_BUILTIN_INIT=0 -DX0X_ARP_HOOKS=0 -DX0X_SYNTHONE_PATCH=0 -DX0X_S1C=0'
fi
[[ $arm == pre ]] || flags="$flags -DSHARED_PAGE_POOL=1"
X0X_HOST_DEFS="$flags" SMC_UI_OUT="$PWD/$name" SMC_UI_TEST_FILE="tests/host/$test.c" sh tests/host/sj6-smc-ui.sh > "$name.log" 2>&1
