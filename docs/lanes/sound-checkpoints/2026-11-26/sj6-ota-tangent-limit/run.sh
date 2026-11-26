#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")/../.." && pwd); cd "$D"; X=$D/sj6-ota-history/tangent; L=$D/sj6-raw-main-ota
W=/tmp/phaser-warm-own.Y0XNbQ/tree/firmware/src
O=$(mktemp -d "$X/tmp.XXXXXX"); trap 'rm -rf "$O"' EXIT; export TMPDIR=$O
python3 "$D/sj6-ota-history/wire.py" "$L/physical.c" "$O/packed.c"
python3 "$X/wire.py" "$O/packed.c" "$O/tangent.c"
python3 "$L/make.py" "$O/stock.c"
cc -O2 -fPIC -shared -ffp-contract=off -DX0X_SJ6_OTA_NATIVE=1 -I"$W/dsp" -I"$W" "$O/stock.c" -lm -o "$O/stock.so"
for n in packed tangent; do cc -O2 -fPIC -shared -ffp-contract=off "$O/$n.c" -lm -o "$O/$n.so"; done
python3 - "$O/auth.py" <<'PY'
from pathlib import Path
import sys
s=Path('sj6-ota-history/tanhreuse/auth.py').read_text()
s=s[:s.index('calls={')]+s[s.index('from scipy.signal import hilbert'):]
s=s.replace('shared.reached_tanh_peak.restype=C.c_float','')
s=s.replace('reached_tanh_peak=shared.reached_tanh_peak(),vs_exact=quality,calls=calls,','vs_exact=quality,')
s=s.replace('actual_knob_internal_range=','signed_harmonics_vs_original=[dict(angle_deg=float(np.angle(a/b,deg=True)),amp_db=20*float(np.log10(abs(a/b)))) for a,b in zip(h@model[i,l:r],hy)], actual_knob_internal_range=')
Path(sys.argv[1]).write_text(s)
PY
export PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench OPENBLAS_NUM_THREADS=1
for c in authored lower; do python3 "$O/auth.py" "$O/stock.so" "$O/tangent.so" "$O/packed.so" "$c" > "$X/$c.json"; done
python3 "$X/stress.py" "$O/packed.so" "$O/tangent.so" > "$X/stress.json"
