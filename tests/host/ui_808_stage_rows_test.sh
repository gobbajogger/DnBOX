#!/bin/sh
# Read-only target laws, actual owner clocks, full rows and real negative controls.
set -eu
cd "$(dirname "$0")/../.."
OUT=${1:-build/host/ui-808-stage-rows}
mkdir -p "$OUT"
sh host/build_host.sh --generate-only > "$OUT/generate.log" 2>&1
for name in drum808_fixed_attack drum808_fixed_curve; do
 ${CC:-cc} -O1 -ffp-contract=off -std=c99 -Ifirmware/src/dsp -Ibuild/gen \
  "tests/host/${name}_test.c" firmware/src/dsp/drum808.c firmware/src/dsp/x0x_param.c -lm -o "$OUT/$name"
 "$OUT/$name"
done
SMC_UI_OUT="$OUT/engine" SMC_UI_TEST_FILE=tests/host/ui_fixed_ma_owner_test.c sh tests/host/smc_ui_test.sh
SMC_UI_OUT="$OUT/rows" SMC_UI_TEST_FILE=tests/host/ui_808_stage_rows_test.c sh tests/host/smc_ui_test.sh
python3 tests/host/ui_808_stage_rows_mutants.py
