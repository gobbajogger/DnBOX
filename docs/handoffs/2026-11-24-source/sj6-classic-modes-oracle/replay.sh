#!/bin/sh
set -eu
W=${1:-$PWD}
D=$W/perf-final-proof/sj6-classic-modes-oracle
P=$W/perf-final-proof/sj6-legacy-chorus-core
R=$W/perf-final-proof/sj6-original-chorus-control/source-witness.py
export PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=$W/sj6-private-final/bench-work/sj6-fullbench/bench
python3 "$D/dispatch-modes.py" "$W/perf-final-proof/sj6-layer-tuning-input/SuperJ6-AU-original-extension.private.zip" "$R"
t=$(mktemp /tmp/sj6-classic-modes.XXXXXX)
trap 'rm -f -- "$t"' EXIT
cc -O2 -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math -fsanitize=undefined -fno-sanitize-recover=all -shared -fPIC -I"$P" "$P/classic.c" "$D/wrapper.c" -lm -o "$t"
python3 "$D/extracted-oracle.py" "$t"
