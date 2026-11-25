#!/usr/bin/env bash
set -euo pipefail
H="$(cd "$(dirname "$0")" && pwd)"; R="$(cd "$H/../.." && pwd)"
case "$1" in owner-epoch-link-wet|delay-center-exact-wet) arm="$1";; *) exit 2;; esac
case "$2" in elite) model=elite; board=1;; pro) model=pro; board=2;; *) exit 2;; esac
P="$(cat "$H/$arm.projection")"; [[ "$P" == /tmp/dnbox-emulator-only-* ]]
seed="$P/$model-authored-nor.bin"; [[ $(stat -c %s "$seed") == 1048576 ]]
emu="$R/restored-gate/fm1-live-ba83317a"
[[ $(sha256sum "$emu"|cut -c1-64) == fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017 ]]
elf="$R/rehydrated-reverb/$arm/tree/labout/$model/never-install.elf"
[[ -f "$elf" ]]
dir=$(mktemp -d /tmp/dnbox-delay-centered.XXXXXXXX)
echo "$dir" > "$H/$arm-$model.output"
exec 9>/tmp/dnbox-gate-firmware.lock
flock -x 9
run(){
  local stage="$1" shot="$2" cmd="$3"
  mkdir -p "$dir/$stage/work"; cp "$seed" "$dir/$stage/disposable.bin"
  (cd "$dir/$stage"; env FM1_STRICT=1 FM1_NO_SKIP=1 FM1_USB_SEND="$cmd" FM1_USB_TRACE=1 FM1_DNBOX_PCM="$dir/$stage/audio.s16le" FM1_DUMPRAM="$dir/$stage/ram.bin" "$emu" --fwsc "$R/out/DnBOX-0.0.27-SMK37-$model-73749f55-test.fwsc" --mhz 360 --usb --shot "$shot" --flash "$dir/$stage/disposable.bin" > run.log 2>&1)
}
run open 1680 "$(node "$H/wet-controls.mjs" open)"
run adopt 2400 "$(node "$H/wet-controls.mjs" adopt "$dir/open/run.log")"
cmd=$(node "$H/wet-controls.mjs" wet "$dir/open/run.log" "$dir/adopt/run.log")
printf '%s\n' "$cmd" > "$H/$arm-$model.commands"
run wet 8000 "$cmd"
node "$H/wet-controls.mjs" receipt "$dir/open/run.log" "$dir/wet/run.log" "$board" > "$H/$arm-$model.controls.json"
python3 "$R/remote-typed-load/delivery/read-telemetry.py" "$elf" "$dir/wet" > "$H/$arm-$model.timing.json"
python3 "$H/delay-witness.py" "$elf" "$dir/wet/ram.bin" "$H/delay-layout.s" > "$H/$arm-$model.witness.json"
grep -E "^flash:|^irq levels at the end:" "$dir/wet/run.log" > "$H/$arm-$model.emulator.log"
cmp -s "$seed" "$dir/wet/disposable.bin" || { echo "unexpected virtual NOR write"; exit 4; }
cat "$H/$arm-$model.controls.json" "$H/$arm-$model.timing.json"
