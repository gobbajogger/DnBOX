#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")"; pwd); R=$(dirname "$D")
T=$(mktemp -d /tmp/dnbox-emulator-only-XXXXXXXX)
trap 'rm -rf "$T"' EXIT
exec 9>/tmp/dnbox-gate-firmware.lock
for item in control shadow; do
 case "$item" in
 control)
  aux="$R/s1-morph-native-909txn-compressed-only"; elf="$R/ui-native-expcompact-coldread/tree/widen-native/elite/never-install.elf"; h=31bc1e980e1fb63630b51e5d9bbedfc06efa888062699fed6cf107acd9250f6d;;
 shadow)
  aux="$R/s1-morph-native-909txn-shadowcheck"; elf="$aux/tree/widen-native/elite/never-install.elf"; h=6ed35d508f3241283c35821b4e500872554254fec48dc5bd588100d4a17dc5dc;;
 esac
 seed="$T/$item.bin"; s=$(python3 "$aux/replay-overlay.py" make elite "$elf" "$h" "$seed")
 bash "$aux/run-verified.sh" --plan elite 0 "$elf" "$h" "$seed" "$s"
 mkdir -p "$D/$item"
 python3 "$D/map.py" "$elf" >"$D/$item/observer.map"
 # Keep the original strict guard and exact physical events; collect full run
 # so passive observer perturbation can be checked by its whole-shot PCM hash.
 flock -x 9
 FM1_DNBOX_MAP="$D/$item/observer.map" FM1_DNBOX_TRACE="$D/$item/trace.jsonl" \
   OBS_AUX="$aux" bash "$D/run-observer.sh" --run elite 0 "$elf" "$h" "$seed" "$s" | tee "$D/$item/replay.log"
 flock -u 9
 shot=$(grep '^/tmp/dnbox-loaded-replay-' "$D/$item/replay.log")
 cmp "$seed" "$shot/disposable.bin"
 first=$(grep '^/tmp/dnbox-loaded-replay-' "$aux/elite-replay.log")
 if cmp -s "$shot/audio.s16le" "$first/audio.s16le"; then
   echo 'observer PCM invariant' >>"$D/$item/replay.log"
 else
   echo 'observer changed PCM; own diagnostic only' >>"$D/$item/replay.log"
 fi
done
