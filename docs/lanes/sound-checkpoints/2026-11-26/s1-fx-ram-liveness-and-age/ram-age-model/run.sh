#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd); W=$(cd "$D/../..";pwd)
T=$(mktemp -d "$W/age-work.XXXXX"); trap 'rm -r "$T"' EXIT
REF="$W/s1-fx-parity/reference-build"
S=$(find "$REF" -path '*/AudioKit/Core/Soundpipe/modules/base.c'|head -1)
U=$(find "$REF" -path '*/AudioKit/Core/Soundpipe/modules/revsc.c'|head -1)
H=$(dirname "$(dirname "$S")")
sha256sum "$U" "$W/recovered-sound/s1-room-parity/check/062-room.tap" "$W/s1-organ-wet-recovery/widen-lab/original-room.tap" > "$T/pins"
python "$D/wire-age.py" "$U" "$T/age.c"
sed -e '/^int main/i extern void report(sp_revsc *);' -e '/fclose(out);/s/sp_revsc_destroy/report(q);sp_revsc_destroy/' "$W/s1-organ-wet-recovery/matched-reference.c" > "$T/drive.c"
flags=(-O2 -fno-builtin -fno-fast-math -ffp-contract=off -I"$H" -I"$H/lib/dr_wav" -DNO_LIBSNDFILE)
for arm in full age;do
  if [ "$arm" = full ];then src="$U"; drive="$W/s1-organ-wet-recovery/matched-reference.c";else src="$T/age.c";drive="$T/drive.c";fi
  cc "${flags[@]}" "$drive" "$src" "$S" -lm -o "$T/$arm"
done
python - "$T/live.tap" <<'PY'
import numpy as np,sys
n=264728
t=np.arange(n)/44100.
x=np.zeros((n,10),'<f4')
s=(.2*np.sin(2*np.pi*1037*t)+.13*np.sin(2*np.pi*2913*t))*(t>.10)*(t<4.50)
x[:,2]=s
x[:,3]=-s
x[:,8]=np.select([t<1.5,t<3.5,t<4.8],[.84,.95,.62],default=.92)
x.tofile(sys.argv[1])
PY
for k in pad organ live;do
  case $k in
   pad) in="$W/recovered-sound/s1-room-parity/check/062-room.tap";;
   organ) in="$W/s1-organ-wet-recovery/widen-lab/original-room.tap";;
   live) in="$T/live.tap";;
  esac
  "$T/full" "$in" "$T/$k-full.raw" 1
  "$T/age" "$in" "$T/$k-age.raw" 1 2>"$T/$k-alloc.json"
done
python "$D/score.py" "$T"
