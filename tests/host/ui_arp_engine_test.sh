#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# The arp pages against Synths' real hooks; SKIP until engine_arp.h is in the tree.
set -eu
cd "$(dirname "$0")/../.."
[ -f firmware/src/app/engine_arp.h ] || { echo "SKIP ui-arp-engine (no engine_arp.h)"; exit 0; }
export X0X_SYNTHONE_PATCH=1
X0X_HOST_DEFS="-DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1" SMC_UI_TEST_FILE=tests/host/ui_arp_engine_test.c \
    SMC_UI_OUT=${1:-build/host/ui-arp-engine} sh tests/host/smc_ui_test.sh
