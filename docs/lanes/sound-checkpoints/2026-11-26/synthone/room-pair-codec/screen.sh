#!/bin/bash
set -eu
D=$(cd "$(dirname "$0")";pwd); R=$(cd "$D/../..";pwd)
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
A=$(cat "$D/../local-path.txt")
REF="$R/s1-fx-parity/reference-build"
S=$(find "$REF" -path '*/AudioKit/Core/Soundpipe/modules/base.c'|head -1)
H=$(dirname "$(dirname "$S")")
U=$(find "$REF" -path '*/AudioKit/Core/Soundpipe/modules/revsc.c'|head -1)
python "$D/wire.py" "$U" "$T/revsc.c"
sed -e '/^int main/i extern void room_storage_stats(sp_revsc *);' -e '/fclose(out);/s/sp_revsc_destroy/room_storage_stats(q);sp_revsc_destroy/' "$D/../matched-reference.c" > "$T/drive.c"
for spec in 9-6-8 9-7-8 10-6-8 10-6-12 10-6-16 10-7-16 10-6-16-cubic 11-6-16 11-6-16-cubic;do
 IFS=- read -r l h b pred <<< "$spec"
 extra=''
 [[ ${pred:-} != cubic ]] || extra=-DROOM_CUBIC_PREDICT=1
 cc $extra -std=gnu11 -O2 -fno-builtin -fno-fast-math -ffp-contract=off -I"$H" -I"$H/lib/dr_wav" -DNO_LIBSNDFILE -DROOM_LOW="$l" -DROOM_HIGH="$h" -DBLOCK="$b" "$T/drive.c" "$T/revsc.c" "$S" -lm -o "$T/arm"
 "$T/arm" "$A/original-wet.tap" "$T/$spec.raw" 1 "$T/$spec.phase" 2>"$T/$spec.json"
 cmp "$A/trajectory.raw" "$T/$spec.phase"; rm "$T/$spec.phase"
done
OPENBLAS_NUM_THREADS=1 python "$D/screen.py" "$A/original-wet.tap" "$T"
