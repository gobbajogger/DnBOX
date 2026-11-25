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
[[ "$EHASH" == 02ceb3c1c7a3b032e24a0a149daf3a00238f869c8628ba73975e8fe6815f1147 && "$PHASH" == c2e88ec1e2f8efb87933a7c7f1f3eb25b4e945a2747a3de750cd0af8c5bd1dfe ]]
baseappdir=$(project_nor)
near=${DNBOX_NEAR_BASE:-$R/rehydrated-reverb/near-bound}
coldlog=${DNBOX_COLD_RECEIPT:-$D/near-cold.log}
colddir=$(head -1 "$coldlog")
[[ "$baseappdir" == /tmp/dnbox-emulator-only-* && "$trialdir" == /tmp/dnbox-emulator-only-* &&
   "$colddir" == /tmp/dnbox-emulator-only-* ]]
[[ $(sha256sum "$near/tree/labout/elite/never-install.elf"|cut -c1-64) == 5bcd76a82e44a5ce135157c8ccac3ce21be212c3783d285772588a77585e5594 &&
   $(sha256sum "$near/tree/labout/pro/never-install.elf"|cut -c1-64) == 7145d630918e221c6ef64d7da5d976a3126c42ab4cbd56b0012e1f2b3e77f394 ]]
owner="$R/remote-typed-load/tree/build/smk37-elite/x0x.elf"
owner_ram="$R/remote-typed-load/loaded-smoke-elite/ram.bin"
for model in elite pro; do
    old="$base/tree/labout/$model/never-install.elf"
    new="$trial/tree/labout/$model/never-install.elf"
    loc() { readelf -sW "$1" | awk '$8=="proj"{print $2,$3,$4,$7}'; }
    src="$near/tree/labout/$model/never-install.elf"
    cmp <(loc "$old") <(loc "$new")
    cmp <(loc "$old") <(loc "$src")
    [[ -s "$colddir/$model-offset--1/result.json" && -s "$colddir/$model-offset--1/ram.bin" ]]
    for dir in "$baseappdir" "$trialdir"; do
        common=("$dir/$model-virtual-nor.bin" "$src" "$colddir/$model-offset--1/ram.bin" "$owner" "$owner_ram" "$D/fixture")
        python3 "$D/seed.py" "${common[@]}" "$dir/$model-normal.bin"
        python3 "$D/seed.py" "${common[@]}" "$dir/$model-active.bin" --active
    done
done
printf 'base %s\ntrial %s\n' "$baseappdir" "$trialdir"
USB=$(node "$D/owner-scan.mjs" commands)
for profile in active normal; do
 for model in elite pro; do
  for arm in base trial; do
   dir="$baseappdir"; path="$base"; if [[ "$arm" == trial ]]; then dir="$trialdir"; path="$trial"; fi
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
