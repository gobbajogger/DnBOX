#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
exec 9>/tmp/dnbox-gate-firmware.lock; flock -x 9
T=$(mktemp -d /tmp/dnbox-emulator-only-XXXXXXXX)
trap 'rm -rf "$T"' EXIT
for m in elite pro;do
 p="$D/tree/xip-${FETCH_MODE:-flash}/$m/never-install.elf"
 h=$(sha256sum "$p"|cut -c1-64)
 seed="$T/$m-virtual-nor.bin"
 s=$(python3 "$D/replay-overlay.py" make "$m" "$p" "$h" "$seed")
 bash "$D/run-verified.sh" --plan "$m" 0 "$p" "$h" "$seed" "$s"
 bash "$D/run-verified.sh" --run "$m" 0 "$p" "$h" "$seed" "$s" | tee "$D/$m-${FETCH_MODE:-flash}-replay.log"
done
