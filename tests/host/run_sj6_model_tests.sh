#!/bin/sh
# Local model/protocol tests. No device, full instrument or resource admission.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
OUT=${1:-"$(mktemp -d)"}
mkdir -p "$OUT"
CC=${CC:-cc}
"$CC" -std=c11 -O2 -ffp-contract=off -fno-fast-math -I "$ROOT/firmware/src/dsp" "$ROOT/firmware/src/dsp/sj6_math_model.c" "$ROOT/tests/host/sj6_model_unit_test.c" -lm -o "$OUT/protocol"
"$OUT/protocol"
"$CC" -std=c11 -O2 -ffp-contract=off -fno-fast-math -I "$ROOT/firmware/src/dsp" "$ROOT/firmware/src/dsp/sj6_math_model.c" "$ROOT/tests/host/sj6_model_boundary_test.c" -lm -o "$OUT/boundary"
"$OUT/boundary"
"$CC" -std=c11 -O2 -ffp-contract=off -fno-fast-math -I "$ROOT/firmware/src/dsp" "$ROOT/firmware/src/dsp/sj6_math_model.c" "$ROOT/tools/sj6-model-render.c" -o "$OUT/render"
python3 - "$ROOT" "$OUT" <<'PY'
from pathlib import Path
import hashlib,json,math,struct,subprocess,sys
root,out=map(Path,sys.argv[1:]);m=json.loads((root/'assets/superj6/MODELS.json').read_text())
frames=0
for r in m['models']:
 p=root/r['file'];assert len(p.read_bytes())==r['model_bytes'];assert hashlib.sha256(p.read_bytes()).hexdigest()==r['model_sha256']
 y=out/(r['set']+'-'+r['name']+'.f32');subprocess.run([str(out/'render'),str(p),str(y)],check=True,stdout=subprocess.DEVNULL)
 data=y.read_bytes();assert len(data)==4*r['frames'];assert all(math.isfinite(x[0])for x in struct.iter_unpack('<f',data));assert hashlib.sha256(data).hexdigest()==r['host_O2_output_sha256'];frames+=r['frames']
print('PASS: %d coefficient models, %d complete raw-domain frames, finite and exact retained host vectors; not original-AU/full-instrument acceptance'%(len(m['models']),frames))
PY
