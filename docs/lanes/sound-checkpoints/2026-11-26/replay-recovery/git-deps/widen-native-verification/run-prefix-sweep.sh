#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); cd "$D/.."
model=elite
E=s1-organ-wet-recovery/native-widen-bfp/tree/widen-native/elite/never-install.elf
H=d6c0611a98d75755f2e57b2562fef99f4d4ae671652debcbc3c84469dd2b16b0
S=d9953d21980f6ae7dc26296c8b513618f11389db87beb54430ff7c2d3377ff31
seed=$(sha256sum /tmp/dnbox-emulator-only-*/elite-widen-room.bin 2>/dev/null | awk -v H="$S" '$1==H {print $2;exit}')
[[ -n "$seed" ]]
base=$(grep -m1 '^/tmp/' "$D/loaded.log")
exec 9>/tmp/dnbox-gate-firmware.lock; flock -n 9
for ms in "$@";do
  bash "$D/run-loaded-prefix.sh" --run "$model" 0 "$E" "$H" "$seed" "$S" "$ms" | tee "$D/prefix-$ms.log"
  dir=$(grep -m1 '^/tmp/' "$D/prefix-$ms.log")
  python "$D/prefix-compare.py" "$E" "$base" "$dir" >"$D/prefix-$ms.json"
  python - "$D/prefix-$ms.json" <<'PY'
import json,sys
p=json.load(open(sys.argv[1]))
assert p["pcm_equal_before_terminal_half"] and p["usb_equal_entire_observed_prefix"]
print(p["short_last_midi_ms"],p["short_endpoint"])
PY
done
