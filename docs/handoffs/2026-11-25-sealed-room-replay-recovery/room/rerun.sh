#!/bin/bash
# No package creation, hardware or release. Hold root's timing authorization.
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D")
base=$(realpath "$1"); trial=$(realpath "$2"); offset=${3:-0}
[[ ${DNBOX_SERIAL_EMULATOR_GRANTED:-0} == 1 && "$offset" =~ ^[0-5]$ ]]
exec 9>/tmp/dnbox-gate-firmware.lock
flock -x -n 9
DNR3_WEB="$R/remote206-typed/remote/dnr3.mjs"; export DNR3_WEB
export PYTHONPATH="$R/restored-harness-inspect/helpers"
emu="$R/restored-gate/fm1-live-ba83317a"
project="$R/restored-harness-inspect/helpers/guarded_replay.py"
archive="$R/restored-harness-inspect/dot-gate-harness.zip"
scene="$R/fw/experiments/sound-hardware-v1/strict-timing/heavy.keys"
[[ $(sha256sum "$emu"|cut -c1-64) == fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017 ]]
[[ $(sha256sum "$scene"|cut -c1-64) == d08718b1521119b252b5cca0c5e1db6981ea25a5a0b31b6ead60dceccb3f6271 ]]
bash "$D/build-probes.sh" "$trial/tree"
params() {
    ELITE="$1/tree/labout/elite/never-install.elf"; PRO="$1/tree/labout/pro/never-install.elf"
    EHASH=$(sha256sum "$ELITE"|cut -c1-64); PHASH=$(sha256sum "$PRO"|cut -c1-64)
}
params "$trial"
[[ "$EHASH" == 14560fd643bf571c2e36fdf5e5085d17cc3233b218d1a9bbbfe6b5f2befa2e43 &&
   "$PHASH" == 0612a1ae686b516bd42ffc93e681736d34917f91ba7810c10899261baf8888df ]]
project_nor() {
    python3 "$project" --workspace "$R" --archive "$archive" --elite "$ELITE" --elite-sha "$EHASH" --pro "$PRO" --pro-sha "$PHASH" "$@"
}
trialdir=$(project_nor)
params "$base"
coldlog=$(mktemp)
project_nor --run --offsets=-1 --jobs=1 | tee "$coldlog"
basedir=$(head -1 "$coldlog")
rm "$coldlog"
[[ "$basedir" == /tmp/dnbox-emulator-only-* && "$trialdir" == /tmp/dnbox-emulator-only-* ]]
owner="$R/remote-typed-load/tree/build/smk37-elite/x0x.elf"
owner_ram="$R/remote-typed-load/loaded-smoke-elite/ram.bin"
for model in elite pro; do
    old="$base/tree/labout/$model/never-install.elf"
    new="$trial/tree/labout/$model/never-install.elf"
    loc() { readelf -sW "$1" | awk '$8=="proj"{print $2,$3,$4,$7}'; }
    cmp <(loc "$old") <(loc "$new")
    for dir in "$basedir" "$trialdir"; do
        common=("$dir/$model-virtual-nor.bin" "$old" "$basedir/$model-offset--1/ram.bin" "$owner" "$owner_ram" "$D/fixture")
        python3 "$D/seed.py" "${common[@]}" "$dir/$model-normal.bin"
        python3 "$D/seed.py" "${common[@]}" "$dir/$model-active.bin" --active
    done
done
printf 'base %s\ntrial %s\n' "$basedir" "$trialdir"
USB=$(node "$D/owner-scan.mjs" commands)
for profile in active normal; do
 for model in elite pro; do
  for arm in base trial; do
   dir="$basedir"; path="$base"; if [[ "$arm" == trial ]]; then dir="$trialdir"; path="$trial"; fi
   run="$dir/$model-$profile-$offset"; mkdir "$run"; mkdir "$run/work"
   copied="$run/disposable.bin"; cp "$dir/$model-$profile.bin" "$copied"
   python3 - "$scene" "$run/scene.keys" "$offset" <<'PY'
from pathlib import Path
import sys
src,dst,off=sys.argv[1:]
x=[n.split(':',1) for n in Path(src).read_text().strip().split(',')]
assert len(x)==922
Path(dst).write_text(','.join(f'{int(t)+int(off)}:{k}' for t,k in x)+'\n')
PY
   elf="$path/tree/labout/$model/never-install.elf"
   (
    cd "$run"
    FM1_NO_SKIP=1 FM1_STRICT=1 FM1_BENCH_KEYS="@$run/scene.keys" FM1_USB_SEND="$USB" FM1_USB_TRACE=1 FM1_DNBOX_PCM="$run/audio.s16le" FM1_DUMPRAM="$run/ram.bin"       "$emu" --fwsc "$R/out/DnBOX-0.0.27-SMK37-$model-73749f55-test.fwsc" --mhz 360 --usb --shot 8000 --flash "$copied" >run.log 2>&1
   )
   echo "$arm $model $profile $offset $run"
   node "$D/owner-scan.mjs" receipt "$run/run.log" | tee "$run/owner.json"
   python3 "$R/remote-typed-load/delivery/read-telemetry.py" "$elf" "$run" | tee "$run/telemetry.json"
   python3 "$D/strict-budget.py" "$elf" "$run" "$dir/$model-$profile.bin" | tee "$run/strict-budget.json" || echo "STRICT BUDGET WITNESS FAILED"
   python3 "$D/room-witness.py" "$elf" "$run" "$D/room-offsets.s" "$profile" | tee "$run/room-witness.json" || echo "ROOM WITNESS FAILED"
  done
 done
done
