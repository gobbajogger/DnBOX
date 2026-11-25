#!/bin/bash
# Requires caller-held sole emulator slot. No install or release output.
set -euo pipefail
H=$(cd "$(dirname "$0")" && pwd)
R=$(dirname "$H")
[[ $# = 7 ]]
mode=$1 model=$2 off=$3 elf=$(realpath "$4") elf_sha=$5 seed=$(realpath "$6") seed_sha=$7
[[ $mode == --plan || $mode == --run ]]
[[ $model == elite || $model == pro ]]
[[ $off =~ ^[0-5]$ && $seed == /tmp/dnbox-emulator-only-* && -f $seed ]]
[[ $elf_sha =~ ^[0-9a-f]{64}$ && $seed_sha =~ ^[0-9a-f]{64}$ ]]
bash "$H/verify.sh" >/dev/null
printf '%s  %s\n%s  %s\n' "$elf_sha" "$elf" "$seed_sha" "$seed" | sha256sum --status -c -
[[ $(stat -c %s "$seed") = 1048576 ]]
python3 "$H/check-seed.py" "$model" "$elf" "$elf_sha" "$seed"

export DNR3_WEB="$R/remote206-typed/remote/dnr3.mjs"
usb=$(node "$H/owner-scan.mjs" commands)
if [[ $mode == --plan ]]; then printf '%s %s %s %s\n' "$model" "$off" "$elf_sha" "$seed_sha"; exit; fi
d=$(mktemp -d "/tmp/dnbox-loaded-replay-$model-$off.XXXXXXXX")
mkdir "$d/work"
printf '%s\n' "$d"
cp "$seed" "$d/disposable.bin"
python3 - "$R" "$d" "$off" <<'PY'
from pathlib import Path
import sys
r,d,off=sys.argv[1:]
keys=(Path(r)/'fw/experiments/sound-hardware-v1/strict-timing/heavy.keys').read_text().strip().split(',')
assert len(keys)==922
(Path(d)/'scene.keys').write_text(','.join(f'{int(t)+int(off)}:{k}' for t,k in (x.split(':',1) for x in keys))+'\n')
PY
cd "$d"
env FM1_STRICT=1 FM1_NO_SKIP=1 FM1_BENCH_KEYS="@$d/scene.keys" FM1_USB_SEND="$usb" FM1_USB_TRACE=1 \
    FM1_DNBOX_PCM="$d/audio.s16le" FM1_DUMPRAM="$d/ram.bin" "$R/restored-gate/fm1-live-ba83317a" \
    --fwsc "$R/out/DnBOX-0.0.27-SMK37-$model-73749f55-test.fwsc" --mhz 360 --usb --shot 8000 \
    --flash "$d/disposable.bin" >run.log 2>&1
node "$H/owner-scan.mjs" receipt "$d/run.log" >owner.json
python3 "$H/probe.py" "$elf" "$d" | tee result.json
