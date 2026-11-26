#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$(dirname "$D")")
E="$R/sj6-j60-right-native/out"
T=$(mktemp -d /tmp/dnbox-emulator-only-j60-XXXXXXXX);trap 'rm -rf "$T"' EXIT
sourcepins=(0ffdc7faf4f8ecace4e119accce10ecb64dac16122d2586d323c388da305bd7c ddbcae26a746c6ee2f4ccae382ceb4762d06f42d3544875862dcb75f94c3ea51)
exec 9>/tmp/dnbox-gate-firmware.lock
for i in 0 1;do
 m=elite; [[ "$i" == 0 ]] || m=pro
 elf="$E/warm-$m/1/never-install.elf"; p="${sourcepins[$i]}"
 seed="$T/$m.bin"; s=$(python3 "$D/replay-overlay.py" make "$m" "$elf" "$p" "$seed")
 bash "$D/run-verified.sh" --plan "$m" 0 "$elf" "$p" "$seed" "$s"
 flock -x 9
 bash "$D/run-verified.sh" --run "$m" 0 "$elf" "$p" "$seed" "$s" | tee "$D/$m.log"
 flock -u 9
 shot=$(grep "^$D/dnbox-loaded-replay-" "$D/$m.log")
 cmp "$seed" "$shot/disposable.bin"
 printf 'verified: byte-identical disposable virtual NOR\n' | tee -a "$D/$m.log"
 cmp "$R/fw/experiments/sound-hardware-v1/strict-timing/heavy.keys" "$shot/scene.keys"
 sha256sum "$seed" "$shot/disposable.bin" "$shot/scene.keys" | tee "$D/$m-sha256.txt"
done
