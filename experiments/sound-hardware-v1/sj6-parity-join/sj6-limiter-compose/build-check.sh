#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
D=../sj6-parallel-joined/tree/firmware/src
python stage.py "$D" variant
python - "$D/dsp" <<'PY'
from pathlib import Path
import sys
d=Path(sys.argv[1]); v=Path('variant')
def body(f,start):
 s=f.read_text().split(start)[1]
 return start+s.rsplit('#endif',1)[0]
room='static int16_t sj6_room_pcm'
old=body(d/'sj6_native_room.inc',room).replace('sj6_room_process','old_room')
for n in ('sj6_room_tick','sj6_room_pcm','sj6_fatten_tick'):
 old=old.replace(n,'old_'+n)
lim=body(d/'sj6_native_limiter.inc','static void sj6_limiter_process').replace('sj6_limiter_process','old_limit')
newlim=body(v/'sj6_native_limiter.inc','static inline float sj6_limiter_sample')
newroom=body(v/'sj6_native_room.inc',room)
Path('leaves.h').write_text(old+lim+newlim+newroom)
PY
cc -O2 -ffp-contract=off -DX0X_SJ6_NATIVE_ROOM=1 -DX0X_SJ6_NATIVE_LIMITER=1 -I"$D/dsp" differential.c -lm -o test
./test
