#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
export SMC_UI_TEST_FILE=tests/host/ui_drum_amp_graph_test.c
export SMC_UI_OUT=${SMC_UI_OUT:-build/host/drum-amp-graph}
sh tests/host/smc_ui_test.sh
for mode in DRUM_GRAPH_MISSING DRUM_GRAPH_STATIC; do
 if env "$mode=1" "$SMC_UI_OUT/test" > "$SMC_UI_OUT/$mode.log" 2>&1; then echo "FAIL: $mode falsely passed";exit 1;fi
 grep -q 'graph_hash(ep)!=lo' "$SMC_UI_OUT/$mode.log"
done
echo 'BD/SD missing/static negative controls rejected PASS'
