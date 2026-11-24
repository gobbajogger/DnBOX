#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Keyed roll accent/slide marks (#61), then three ui.c source mutants that must each fail.
set -e
cd "$(dirname "$0")/../.."
. tests/host/original_build.sh
OUT=${OUT:-build/host}
mkdir -p "$OUT/roll_marks" build/gen
M=$(cd "$OUT/roll_marks" && pwd)
sh host/build_host.sh --generate-only >/dev/null
build() { ${CC:-cc} -O2 -ffp-contract=off -std=c99 -w -DX0X_HOST $ORIGINAL_FLAGS $1 -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen \
    tests/host/roll_marks_test.c $ORIGINAL_SOURCES -lm -o "$2"; }
build "" "$OUT/roll_marks/test"
"$OUT/roll_marks/test"
mut() {   # from to name
    python3 - "$OUT/roll_marks/ui-mut.c" "$1" "$2" <<'PY'
import sys
s=open('firmware/src/app/ui.c').read(); a,b=sys.argv[2],sys.argv[3]
assert s.count(a)==1, a
open(sys.argv[1],'w').write(s.replace(a,b))
PY
    build "-DX0X_HOST_UI_SOURCE=\"$M/ui-mut.c\" -I$PWD/firmware/src/app" "$OUT/roll_marks/mut"
    if "$OUT/roll_marks/mut" > "$OUT/roll_marks/mut.log" 2>&1; then echo "MUTANT SURVIVED: $3"; exit 1; fi
    echo "mutant killed: $3"
}
mut '(st->flags & BS_ACCENT) ? col : dim(col, 9);' 'dim(col, 9);' "accent not on the note"
mut 'cv_line(x + 11, y + 2, x + 14, ny + 2, C_WHITE);' 'cv_rect(x + 2, ROLL_BOT + 8 - o, 8, 1, C_WHITE);' "slide as a separate row"
mut 'cv_line(x + 12, y + 2, x + 14, y + 2 + ((ny > y) - (ny < y)) * 2, C_WHITE);' ';' "no boundary cue on step 16"
echo "roll-marks: mutants killed"
