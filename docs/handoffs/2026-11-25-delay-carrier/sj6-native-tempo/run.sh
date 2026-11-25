#!/bin/bash
set -euo pipefail
H="$(cd "$(dirname "$0")" && pwd)"; R="$(cd "$H/../.." && pwd)"
model="$1"; [[ "$model" == elite || "$model" == pro ]]
seed="$(cat "$H/projection")/$model-authored-nor.bin"
[[ "$seed" == /tmp/dnbox-emulator-only-* && $(stat -c %s "$seed") == 1048576 ]]
emu="$R/restored-gate/fm1-live-ba83317a"
[[ $(sha256sum "$emu"|cut -c1-64) == fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017 ]]
elf="$R/rehydrated-reverb/delay-center-exact-wet/tree/labout/$model/never-install.elf"
dir=$(mktemp -d /tmp/dnbox-delay-tempo.XXXXXXXX)
echo "$dir" > "$H/$model.output"
exec 9>/tmp/dnbox-gate-firmware.lock
flock -x 9
run(){
 local phase="$1" shot="$2" cmd="$3"
 mkdir -p "$dir/$phase/work"; cp "$seed" "$dir/$phase/disposable.bin"
 (cd "$dir/$phase"; env FM1_STRICT=1 FM1_NO_SKIP=1 FM1_USB_SEND="$cmd" FM1_USB_TRACE=1 FM1_DNBOX_PCM="$dir/$phase/audio.s16le" FM1_DUMPRAM="$dir/$phase/ram.bin" "$emu" --fwsc "$R/out/DnBOX-0.0.27-SMK37-$model-73749f55-test.fwsc" --mhz 360 --usb --shot "$shot" --flash "$dir/$phase/disposable.bin" >run.log 2>&1)
 cmp "$seed" "$dir/$phase/disposable.bin"
}
run open 1680 "$(node "$H/tempo.mjs" open)"
run profile 2200 "$(node "$H/tempo.mjs" profile "$dir/open/run.log")"
for phase in early wet dry; do
 shot=3900; [[ "$phase" != early ]] || shot=2890
 run "$phase" "$shot" "$(node "$H/tempo.mjs" commands "$dir/open/run.log" "$phase" "$dir/profile/run.log")"
 node "$H/tempo.mjs" receipt "$dir/open/run.log" "$phase" "$dir/$phase/run.log" > "$H/$model-$phase.controls.json"
 python3 "$R/remote-typed-load/delivery/read-telemetry.py" "$elf" "$dir/$phase" > "$H/$model-$phase.timing.json"
 grep -E "^flash:|^irq levels at the end:" "$dir/$phase/run.log" > "$H/$model-$phase.emulator.log"
done
