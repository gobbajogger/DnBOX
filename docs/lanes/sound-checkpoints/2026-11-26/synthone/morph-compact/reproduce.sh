#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D"); cd "$R"
A=recovered-sound/s1-room-parity
W=s1-fx-parity/reference-build/factory-inputs/public-morphs.f32
kind=${1:-shared-tight}; mode=${2:-holdouts}
case "$kind" in
 shared-tight) tol=.00005;;
 shared) tol=.0005;;
 *) exit 3;;
esac
B="$D/private/$kind"
python "$D/encode.py" "$W" "$B" --tol "$tol" --share
mkdir -p "$B/build" "$D/private/tmp"
cp "$A/unified-releasephase/"{s1c.c,render.c,tuning-load.h} "$B/build/"
EXACT_PITCH=1 EXACT_CRUSH_FLOAT=1 EXACT_ENV=1 bash "$D/build.sh" "$A/build-releasephase" "$B/build" "$kind"
cc -fPIC -shared -O2 -ffp-contract=off -DDNBX_MORPH_LATTICE_TEST -I"$A" -I"$B" "$D/morph-random.c" -lm -o "$B/morph.so"
cc -fPIC -shared -O2 -ffp-contract=off "$A/exact-morph.c" -lm -o "$B/float.so"
python "$D/check-lattice.py" "$W" "$B" > "$D/lattice-$kind.json"
export TMPDIR="$D/private/tmp"
if [[ "$mode" == bank ]]; then
 python "$A/bank-compare.py" "$A/unified-releasephase/original" "$B/build/candidate" "$W" "$A/aligned-commands.bin" recovered-sound/s1-bank-replay/original-private/pack.private "$D/full-bank-$kind.jsonl" --same-note-tape --tuning-dir "$A/common-project-tuning" --identity "$A/aligned-bank.json" --jobs 5
else
 bash "$D/holdout.sh" "$kind"
 bash "$D/holdout-worst.sh" "$kind"
fi
