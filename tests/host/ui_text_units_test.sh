#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# FINAL-REFERENCES text/unit fixes, then ui.c source mutants that must each fail.
set -eu
cd "$(dirname "$0")/../.."
OUT=${1:-build/host/ui-text-units}
mkdir -p "$OUT"; M=$(cd "$OUT" && pwd)
export SMC_UI_TEST_FILE=tests/host/ui_text_units_test.c
SMC_UI_OUT="$OUT" sh tests/host/smc_ui_test.sh
mut() {   # from to name
    python3 - "$M/ui-mut.c" "$1" "$2" <<'PY'
import sys
s=open('firmware/src/app/ui.c').read(); a,b=sys.argv[2],sys.argv[3]
assert s.count(a)==1, a
open(sys.argv[1],'w').write(s.replace(a,b))
PY
    if SMC_UI_OUT="$OUT/mut" X0X_HOST_DEFS="-DX0X_HOST_UI_SOURCE=\"$M/ui-mut.c\"" sh tests/host/smc_ui_test.sh > "$M/mut.log" 2>&1
    then echo "MUTANT SURVIVED: $3"; exit 1; fi
    grep -q "^FAIL" "$M/mut.log" || { echo "mutant did not build/run: $3"; tail -5 "$M/mut.log"; exit 1; }
    echo "mutant killed: $3"
}
mut '"DIRECTION", "TRANSPOSE"};' '"DIRECTION", "TRANSP"};' "TRANSP on SEQ"
mut 'case FX_RV_HPF: return FX_STRIP ? "LOW CUT" : "REV LOW CUT";' 'case FX_RV_HPF: return "REV LOW CUT";' "REV LOW CUT overruns its strip"
echo "ui-text-units: mutants killed"
