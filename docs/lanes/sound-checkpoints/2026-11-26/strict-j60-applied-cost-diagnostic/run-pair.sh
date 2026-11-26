#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$(dirname "$D")")
E="$D/../link-receipts"
T=$(mktemp -d "$D"/dnbox-emulator-only-j60-XXXXXXXX);trap 'rm -rf "$T"' EXIT
sourcepins=(57b617fa7366b51e22470fa81e9ad0f6916c2de9ebb03571ced8eeb3b42d9c61 476017c9a395cd9bcd596a322987ca0894eb300f0962db9dda1e6a4b7cb30f07)
exec 9>/tmp/dnbox-gate-firmware.lock
for i in 0 1;do
 m=elite; [[ "$i" == 0 ]] || m=pro
 elf="$E/$m-on.never-install.elf"; p="${sourcepins[$i]}"
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
 python "$D/read-busy.py" "$m" "$p" "$elf" "$shot" | tee "$D/$m-busy.json"
done
