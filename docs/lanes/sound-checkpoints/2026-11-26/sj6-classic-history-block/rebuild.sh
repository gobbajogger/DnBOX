#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D"); B=${1:-$R/sj6-hp-connected-zero/host-donor}
X=$(mktemp -d); trap 'rm -r "$X"' EXIT
cd "$D"
cc -O2 -ffp-contract=off -I"$B/firmware/src/dsp" run.c -lm -o "$X/probe"
RAW_DIR="$X" bash extract.sh "$B" > extraction.log
"$X/probe" "$X"/raw-* | tee all.log
