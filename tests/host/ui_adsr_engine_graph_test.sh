#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
export SMC_UI_TEST_FILE=tests/host/ui_adsr_engine_graph_test.c
export SMC_UI_OUT=${SMC_UI_OUT:-build/host/adsr-engine}
sh tests/host/smc_ui_test.sh
for mode in ADSR_MISSING_NEGATIVE ADSR_STATIC_NEGATIVE ADSR_CURVE_NEGATIVE; do
 if env "$mode=1" "$SMC_UI_OUT/test" > "$SMC_UI_OUT/$mode.log" 2>&1; then
  echo "FAIL: $mode falsely passed" >&2;exit 1
 fi
 case "$mode" in
 ADSR_MISSING_NEGATIVE) grep -q 'curve_lit>0' "$SMC_UI_OUT/$mode.log" ;;
 ADSR_CURVE_NEGATIVE) grep -q 'segment_ink>0' "$SMC_UI_OUT/$mode.log" ;;
 ADSR_STATIC_NEGATIVE) grep -q 'graph(p)!=lo' "$SMC_UI_OUT/$mode.log" ;;
 esac
done
echo 'Missing/static/curve-removal full-row graph negatives rejected PASS'
