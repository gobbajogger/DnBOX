#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
GFINAL_REFERENCE_DIR=${GFINAL_REFERENCE_DIR:-tests/reference/gfinal}
OUT=${SMC_UI_OUT:-build/host/ui-chance-framebuffer}
mkdir -p "$OUT"
python3 - "$GFINAL_REFERENCE_DIR" "$OUT/oracle.rgb565" <<'PY'
import hashlib,importlib.util,pathlib,struct,sys
if not __debug__: raise RuntimeError('qualification requires Python assertions enabled')
p=pathlib.Path(sys.argv[1])/'gfinal.py'
assert hashlib.sha256(p.read_bytes()).hexdigest()=='b5daa51981ac1a85a6167f2cdd00f7ae4b9513e6d63bd0019600a6f171ff8b10'
spec=importlib.util.spec_from_file_location('canonical_gfinal',p);g=importlib.util.module_from_spec(spec);spec.loader.exec_module(g)
with open(sys.argv[2],'wb') as out:
 for v in range(101):
  image=g.render('CHANCE',(v*127+50)//100,{},cell='strip')
  for r,gg,b in image.get_flattened_data():out.write(struct.pack('<H',(r>>3)<<11|(gg>>2)<<5|b>>3))
assert pathlib.Path(sys.argv[2]).stat().st_size==101*40*24*2
PY
CHANCE_ORACLE="$PWD/$OUT/oracle.rgb565" SMC_UI_OUT="$OUT" \
SMC_UI_TEST_FILE=tests/host/ui_chance_framebuffer_test.c sh tests/host/smc_ui_test.sh
