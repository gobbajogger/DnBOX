#!/bin/sh
set -eu
W=${1:-$PWD}
D=$W/perf-final-proof/sj6-classic-modes-oracle
P=$W/perf-final-proof/sj6-legacy-chorus-core
t=$(mktemp -d /tmp/sj6-modes-red.XXXXXX)
trap 'rm -rf -- "$t"' EXIT
sed '/\/\* Earlier mode offsets/,+2c\        const float width=s->mode_ii==1 ? .015f : s->mode_ii==2 ? .02f : .04f;\n        const float fb[]={feedback+fmaf(a,.05f,1.f)*width,feedback+fmaf(b,.05f,1.f)*-width};' "$D/wrapper.c" > "$t/prior.c"
cc -O2 -ffp-contract=off -fno-fast-math -shared -fPIC -I"$D" -I"$P" "$P/classic.c" "$t/prior.c" -lm -o "$t/lib.so"
if python3 "$D/extracted-oracle.py" "$t/lib.so" > "$t/receipt" 2>&1; then
  echo 'FAIL oracle did not reject previous additive formula'; exit 1
fi
grep -E 'AssertionError: \(0, [0-9]+, [0-9]+, .[LR].\)' "$t/receipt"
echo 'PASS exact independent PCM oracle rejects prior additive law even before mode II'
