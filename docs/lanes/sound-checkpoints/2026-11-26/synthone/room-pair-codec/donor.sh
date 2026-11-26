#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd);R=$(cd "$D/../..";pwd);cd "$R"
A="$R/recovered-sound/s1-room-parity"
T=$(mktemp -d);trap 'rm -rf "$T"' EXIT
B="$A/unified2"
C="$A/build-lfo"
(cd "$C";sha256sum -c "$B/donor-source.sha256") >&2
for f in render.c s1c.c tuning-load.h;do cp "$B/$f" "$T/";done
MOD=$(find "$R/s1-fx-parity/reference-build" -path '*/AudioKit/Core/Soundpipe/modules/revsc.c' -print -quit)
python "$D/wire.py" "$MOD" "$T/revsc.c"
python - "$A/build-unified-candidate.sh" "$T/build.sh" "$A" <<'PY'
import sys
from pathlib import Path
p,o,a=sys.argv[1:]
s=Path(p).read_text()
s=s.replace('D=$(cd "$(dirname "$0")" && pwd)',f'D={a}')
s=s.replace('cc -std=gnu11','cc ${ROOM_FLAGS:-} -std=gnu11')
s=s.replace('"$M/"{revsc,compressor,vdelay,pan2,phaser}.c','"${ROOM_SRC:-$M/revsc.c}" "$M/"{compressor,vdelay,pan2,phaser}.c')
Path(o).write_text(s)
PY
export EXACT_PITCH=1 EXACT_CRUSH_FLOAT=1 EXACT_ENV=1
run(){
 OPENBLAS_NUM_THREADS=1 python "$A/bank-compare.py" "$B/original" "$1" \
  "$R/s1-fx-parity/reference-build/factory-inputs/public-morphs.f32" "$A/aligned-commands.bin" \
  "$R/recovered-sound/s1-bank-replay/original-private/pack.private" "$2" \
  --same-note-tape --identity "$A/aligned-bank.json" --tuning-dir "$A/common-project-tuning" \
  --ids 0 62 113 154 369 527 788 848 --jobs 2
}
bash "$T/build.sh" "$C" "$T"
run "$T/candidate" "$T/base.jsonl"
for spec in 9-6-8 9-7-8 10-6-8 10-6-12 10-6-16 10-7-16 10-6-16-cubic 11-6-16 11-6-16-cubic;do
 IFS=- read -r l h b pred <<< "$spec"
 extra=''
 [[ ${pred:-} != cubic ]] || extra=-DROOM_CUBIC_PREDICT=1
 ROOM_FLAGS="$extra -DROOM_LOW=$l -DROOM_HIGH=$h -DBLOCK=$b" ROOM_SRC="$T/revsc.c" bash "$T/build.sh" "$C" "$T"
 run "$T/candidate" "$T/$spec.jsonl"
done
python "$D/donor-score.py" "$T" "$A/bank-unified-shared-a4.jsonl"
