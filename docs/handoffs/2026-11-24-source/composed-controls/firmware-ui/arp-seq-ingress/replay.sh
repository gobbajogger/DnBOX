#!/usr/bin/env bash
# Local host-only, no packaging or devices.
set -euo pipefail
cd "$(dirname "$0")"
p=firmware/src/app/ui.c
[[ $(sha256sum "$p"|cut -d' ' -f1) = 556b804412e1560a6c481125ab915d1b68d7849ab167de7547d7ed27d30aa40c ]]
if [[ ${1:-stock} == compact ]]; then
    export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1
    export X0X_HOST_DEFS="-DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -DX0X_ARP_HOOKS=1"
    export SMC_UI_OUT="$PWD/compact-proof"
else
    export X0X_HOST_DEFS="-DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1"
    export SMC_UI_OUT="$PWD/proof"
fi
X0X_SYNTHONE_PATCH=1 SMC_UI_TEST_FILE=tests/host/synth-arp-panel-ingress.c bash tests/host/smc_ui_test.sh
