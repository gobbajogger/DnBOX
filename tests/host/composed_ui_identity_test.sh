#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
OUT=${SMC_UI_OUT:-build/host/composed-ui}
SMC_UI_TEST_FILE=tests/host/composed_ui_identity_test.c SMC_UI_OUT="$OUT" sh tests/host/smc_ui_test.sh
for kind in battery title; do
 NEG="$OUT/negative-$kind"
 mkdir -p "$NEG"
 python3 - "$kind" "$NEG/ui.c" <<'PYTHON'
import pathlib,sys
s=pathlib.Path('firmware/src/app/ui.c').read_text()
old,new,count = ('"S1 A"','"BAD A"',3) if sys.argv[1]=='title' else ('dv_header_battery(219, 5);','(void)0; /* negative: omitted battery */',2)
if s.count(old)!=count: raise SystemExit('production mutation anchor changed')
pathlib.Path(sys.argv[2]).write_text(s.replace(old,new))
PYTHON
 MUTANT=$(cd "$NEG" && pwd)/ui.c
 set +e
 X0X_HOST_DEFS="${X0X_HOST_DEFS:-} -DX0X_HOST_UI_SOURCE=\"$MUTANT\"" SMC_UI_TEST_FILE=tests/host/composed_ui_identity_test.c SMC_UI_OUT="$NEG" sh tests/host/smc_ui_test.sh > "$OUT/negative-$kind.log" 2>&1
 rc=$?
 set -e
 [ "$rc" != 0 ] || { echo "FAIL: production $kind mutation was accepted"; exit 1; }
 case "$kind" in battery) marker=CURRENT_BATTERY_MISMATCH;;title) marker=CURRENT_TITLE_MISMATCH;;esac
 grep -q "$marker" "$OUT/negative-$kind.log" || { cat "$OUT/negative-$kind.log"; exit 1; }
done
echo 'Current identity production negatives: missing battery and wrong title rejected'
