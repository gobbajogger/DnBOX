#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
export SMC_UI_TEST_FILE=tests/host/ui_layer_restoration_test.c
export SMC_UI_OUT=${SMC_UI_OUT:-build/host/ui-layer-restoration}
mkdir -p "$SMC_UI_OUT"
sh tests/host/smc_ui_test.sh > "$SMC_UI_OUT/positive.log" 2>&1
cat "$SMC_UI_OUT/positive.log"
if LAYER_MUTATE_RESTORE=1 "$SMC_UI_OUT/test" > "$SMC_UI_OUT/negative.log" 2>&1; then
    echo 'FAIL damaged background restoration was accepted'; exit 1
fi
python3 - "$SMC_UI_OUT/negative.log" <<'PY'
import pathlib,sys
log=pathlib.Path(sys.argv[1]).read_text()
assert 'LAYER_RESTORE_MISSING pixels=1' in log and 'ui_layer_restoration_test.c:' in log
print('PASS one-pixel restored-background runtime negative')
PY
python3 - "$SMC_UI_OUT" <<'PYCODE'
from pathlib import Path
import hashlib,json,sys
out=Path(sys.argv[1]);root=Path.cwd();sources={}
for name in ['firmware/src/app/ui.c','firmware/src/gfx.c','firmware/src/app/smc_mixer.c','tools/gen_font.py']:
 data=(root/name).read_bytes();sources[name]=hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()
positive=(out/'positive.log').read_text();negative=(out/'negative.log').read_text()
frames=[]
for line in positive.splitlines():
 if line.startswith('LAYER_STATE '):
  state=json.loads(line[12:]);image=out/(state['id']+'.png');frames.append({'state':state,'png':image.name,'sha256':hashlib.sha256(image.read_bytes()).hexdigest()})
assert len(frames)==3

assert 'PASS actual tempo overlay bounds and full base-main restoration (' in positive
assert 'LAYER_RESTORE_MISSING pixels=1' in negative
proof={'status':'PASS','sources':sources,'frames':frames,'popupBounds':[8,103,232,153],'outsidePopupChanges':0,'baseMainRestoration':'BYTE_IDENTICAL','damagedRestorationRejected':True,'positiveLogSHA256':hashlib.sha256(positive.encode()).hexdigest(),'negativeLogSHA256':hashlib.sha256(negative.encode()).hexdigest()}
(out/'layer-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
PYCODE
