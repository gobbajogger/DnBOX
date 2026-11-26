#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$(dirname "$D")")
E="$D/../link-receipts"
T=$(mktemp -d "$D"/dnbox-emulator-only-j60-XXXXXXXX);trap 'rm -rf "$T"' EXIT
sourcepins=(4938a8dff6f6352f3e790bf86b6466be3c1f429b2be670ba51a813d7cfc19038 14a7355f11675e3c65608c65c893fe3202724484ec15b41f48511fe9de9d4973)
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
done
