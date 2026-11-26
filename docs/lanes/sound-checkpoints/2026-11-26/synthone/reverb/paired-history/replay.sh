#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd);R=$(cd "$D/../..";pwd)
A=$1;umask 077;export OPENBLAS_NUM_THREADS=1
T=$(mktemp -d);trap 'rm -rf "$T"' EXIT
REF="$R/s1-fx-parity/reference-build"
S=$(find "$REF" -path '*/AudioKit/Core/Soundpipe/modules/base.c'|head -1)
H=$(dirname "$(dirname "$S")")
U=$(find "$REF" -path '*/AudioKit/Core/Soundpipe/modules/revsc.c'|head -1)
python "$D/wire.py" "$U" "$T/revsc.c"
sed -e '/^int main/i extern void room_storage_stats(sp_revsc *);' -e '/fclose(out);/s/sp_revsc_destroy/room_storage_stats(q);sp_revsc_destroy/' "$D/../matched-reference.c" > "$T/drive.c"
for pair in 64-64 16-16 16-12 16-8 12-8 8-8 h16-16 h16-12 h16-8 h16-6 14-4 13-4 12-5 11-6;do
 lo=${pair%-*};hi=${pair#*-}
 extra=();
 if [[ $lo == h16 ]]; then lo=16;extra=(-DROOM_H16=1);fi
 if (( hi<=6 )); then extra+=(-DROOM_FRONTIER=1);fi
 cc -std=gnu11 -O2 -fno-builtin -fno-fast-math -ffp-contract=off -I"$H" -I"$H/lib/dr_wav" -DNO_LIBSNDFILE -DROOM_LOW=$lo -DROOM_HIGH=$hi "${extra[@]}" "$T/drive.c" "$T/revsc.c" "$S" -lm -o "$T/arm"
 "$T/arm" "$A/original-wet.tap" "$T/out-$pair.raw" 1 "$T/phase-$pair.raw" 2>"$T/out-$pair.stats"
done
python "$D/score.py" "$A" "$T"
