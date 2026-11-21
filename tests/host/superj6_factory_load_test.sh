#!/bin/sh
# Pass the directory containing a bank_data.inc produced by compile_bank.py
# from the authenticated factory archive to exercise all 640 programs as well.
set -eu
cd "$(dirname "$0")/../.."
defs="-DX0X_SUPERJ6_FACTORY=1 -Wl,--wrap=juno6_init"
if [ "$#" -eq 1 ]; then defs="$defs -DSJ6_AUTHORED_BANK -I$1"; fi
export X0X_SUPERJ6_FACTORY=1
export X0X_HOST_DEFS="$defs"
export SMC_UI_TEST_FILE=tests/host/superj6_factory_load_test.c
export SMC_UI_OUT="${SMC_UI_OUT:-build/host/superj6-factory-load}"
exec sh tests/host/smc_ui_test.sh
