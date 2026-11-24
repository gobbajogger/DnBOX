#!/usr/bin/env bash
set -euo pipefail
test "$#" -ge 2 || { echo 'usage: run-combined-controls.sh CANDIDATE SCRATCH [AUTHORIZED_S1_PACK]' >&2; exit 2; }
candidate=$(realpath "$1")
out=$(realpath "$2")
if (($# >= 3)); then export S1_EDITOR_TEST_PACK=$(realpath "$3"); fi
mkdir -p "$out"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
cd "$candidate"
failed=0
for t in ui_superj6_vibrato_test ui_superj6_chorus_test ui_superj6_pwm1_lfo2_test ui_superj6_saga_test ui_superj6_sync_test ui_synthone_sync_storage_test ui_synthone_filter_mode_test ui_arp_engine_test ui_arp_keyboard_test; do
    log="$out/$t.log"
    if SMC_UI_TEST_FILE="tests/host/$t.c" SMC_UI_OUT="$out/$t" bash tests/host/sj6-smc-ui.sh >"$log" 2>&1; then
        echo "$t: $(grep -E 'PASS | checks$' "$log" | tail -n1)"
    else
        echo "FAIL $t ($log)" >&2
        tail -n12 "$log" >&2
        failed=1
    fi
done
exit "$failed"
