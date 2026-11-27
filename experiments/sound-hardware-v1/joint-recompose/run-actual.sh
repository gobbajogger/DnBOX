#!/bin/bash
# Reconstructed diagnostic, emulator only; no transport or historical ELF equivalence claim.
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(cd "$D/../.." && pwd)
W=$(head -1 "$D/cold.log"); [[ $W == /tmp/dnbox-emulator-only-* ]]
bash "$R/recovered-heavy-replay/verify.sh" >/dev/null
exec 9>/tmp/dnbox-gate-firmware.lock; flock -x 9
export DNR3_WEB="$R/remote206-typed/remote/dnr3.mjs"
usb=$(node "$D/owner-scan.mjs" commands)
keys="$R/fw/experiments/sound-hardware-v1/strict-timing/heavy.keys"
for m in elite pro; do
 (
  X=$(mktemp -d "/tmp/dnbox-two-engine-$m.XXXXXXXX")
  echo "$X" > "$D/replay-$m.path"
  elf="$D/tree/joint-diagnostic-later/$m/never-install.elf"
  seed="$W/$m-two-owners.bin"
  [[ $(stat -c %s "$seed") == 1048576 ]]
  cp "$seed" "$X/disposable.bin"
  mkdir "$X/work"; cd "$X"
  env FM1_STRICT=1 FM1_NO_SKIP=1 FM1_BENCH_KEYS="@$keys" FM1_USB_SEND="$usb" FM1_USB_TRACE=1 \
    FM1_DNBOX_PCM="$X/audio.s16le" FM1_DUMPRAM="$X/ram.bin" "$R/restored-gate/fm1-live-ba83317a" \
    --fwsc "$R/out/DnBOX-0.0.27-SMK37-$m-73749f55-test.fwsc" --mhz 360 --usb --shot 8000 \
    --flash "$X/disposable.bin" > run.log 2>&1
  cmp "$seed" "$X/disposable.bin"
  node "$D/owner-scan.mjs" receipt run.log > owner.json
  python3 "$R/recovered-heavy-replay/probe.py" "$elf" "$X" > result.json
  python3 "$D/cue.py" "$R" "$elf" "$X" | tee "$D/replay-$m.json"
 ) & pids+=("$!")
done
rc=0; for pid in "${pids[@]}"; do wait "$pid" || rc=1; done; exit "$rc"
