#!/bin/bash
set -euo pipefail
d=$(realpath "$(dirname "$0")")
if [[ ! -d "$d/source" ]]; then
  base=${DNBOX_EXACT_BASE:-"$d/../controls-init-only/source-s1-detune"}
  mkdir "$d/source"
  cp -a "$base"/{assets,ble,build,experiments,firmware,host,tests,tools} "$d/source/"
  cp "$d/provider-fixture.c" "$d/source/tests/host/provider-fixture.c"
  cp "$d/product-off-fixture.c" "$d/source/tests/host/product-off-fixture.c"
  (cd "$d/source"; patch -s -p1 --fuzz=0 < "$d/amp-provider.patch")
fi
cd "$d/source"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -Itests/host'
if [[ ${1:-enabled} == enabled ]]; then
 export X0X_HOST_DEFS="$X0X_HOST_DEFS -DX0X_SJ6_NOTE_AMP_RECEIVER=1 -Wl,--wrap=juno6_init"
 export SMC_UI_TEST_FILE=tests/host/provider-fixture.c
elif [[ ${1:-} == no-provider ]]; then
 export X0X_HOST_DEFS="$X0X_HOST_DEFS -DX0X_SJ6_NOTE_AMP_RECEIVER=1"
 export SMC_UI_TEST_FILE=tests/host/product-off-fixture.c
else
 export SMC_UI_TEST_FILE=tests/host/product-off-fixture.c
fi
SMC_UI_OUT="$d/host-${1:-enabled}" sh tests/host/sj6-smc-ui.sh > "$d/${1:-enabled}.log" 2>&1
tail -4 "$d/${1:-enabled}.log"
