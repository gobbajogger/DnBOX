#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
[[ $# = 3 ]]
elfdir=$(realpath "$1")
exec 9>/tmp/dnbox-gate-firmware.lock
T=$(mktemp -d /tmp/dnbox-emulator-only-XXXXXXXX)
trap 'rm -rf "$T"' EXIT
for m in ${MODELS:-elite pro};do
 p="$elfdir/$m/never-install.elf"
 if [[ "$m" = elite ]]; then h="$2"; else h="$3"; fi
 seed="$T/$m-virtual-nor.bin"
 s=$(python3 "$D/replay-overlay.py" make "$m" "$p" "$h" "$seed")
 bash "$D/run-verified.sh" --plan "$m" 0 "$p" "$h" "$seed" "$s"
 flock -x 9
 if [[ $m == elite ]]; then export FM1_TPROF="$D/elite-pc.csv,7823,7840"; else export FM1_TPROF="$D/pro-pc.csv,5107,5124"; fi
 bash "$D/run-verified.sh" --run "$m" 0 "$p" "$h" "$seed" "$s" | tee "$D/$m-replay.log"
 flock -u 9
 shotdir=$(grep "^/tmp/dnbox-loaded-replay-" "$D/$m-replay.log")
 cmp "$seed" "$shotdir/disposable.bin"
 printf 'verified: unchanged protected disposable NOR\n' | tee -a "$D/$m-replay.log"
done
