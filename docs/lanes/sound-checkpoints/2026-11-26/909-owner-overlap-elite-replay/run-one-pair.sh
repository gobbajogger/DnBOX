#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd); O=$(dirname "$D"); R=$(dirname "$O")
H="$R/recovered-heavy-replay"
mode=${1:?--plan or --run}; [[ $mode == --plan || $mode == --run ]]
# One board, one pair only: the exact pinned Elite sibling arm.
board=elite
pins=(aa2cacb33abf17bf35b287db214f82185f0bbc8a226f2daad63fb814ce66ac69 16253badc055ad7bcae4a43ad0e091df445e3e4c3448ae900fa775529a2a917d)
T=$(mktemp -d "$D/dnbox-emulator-only-pair.XXXX")
trap 'rm -rf "$T"' EXIT
export PYTHONDONTWRITEBYTECODE=1 DNR3_WEB="$R/remote206-typed/remote/dnr3.mjs"
bash "$H/verify.sh"
keys="$R/fw/experiments/sound-hardware-v1/strict-timing/heavy.keys"
USB=$(node "$H/owner-scan.mjs" commands)
echo "readonly owner probes: $USB"
avail=$(df --output=avail -k "$D" | tail -1)
[[ "$avail" -ge 6800 ]]
for i in 0 1;do
 q=off; [[ "$i" == 0 ]] || q=on
 E="$T/$q.elf"; S="$T/$q.bin"
 gzip -dc "$O/thin-receipts/$board-$q.never-install.elf.gz" >"$E"
 printf '%s  %s\n' "${pins[$i]}" "$E" | sha256sum -c -
 python3 "$D/fault.py" "$E" "$T" "$q" preflight
 sha=$(python3 "$D/pair-overlay.py" make "$board" "$E" "${pins[$i]}" "$S")
 printf '%s  %s\n' "$sha" "$S" | sha256sum -c -
 python3 "$D/pair-overlay.py" check "$board" "$E" "${pins[$i]}" "$S"
 echo "preflight $board $q ELF ${pins[$i]}, seed $sha, 922 original events"
 rm "$E" "$S"
done
[[ $mode == --run ]] || exit 0
exec 9>/tmp/dnbox-gate-firmware.lock
flock -w 60 -x 9
for i in 0 1;do
 q=off; [[ "$i" == 0 ]] || q=on
 E="$T/$q.elf"; S="$T/$q.bin"; shot="$T/shot"
 gzip -dc "$O/thin-receipts/$board-$q.never-install.elf.gz" >"$E"
 sha=$(python3 "$D/pair-overlay.py" make "$board" "$E" "${pins[$i]}" "$S")
 printf '%s  %s\n' "$sha" "$S" | sha256sum -c -
 python3 "$D/pair-overlay.py" check "$board" "$E" "${pins[$i]}" "$S"
 mkdir "$shot" "$shot/work"; cp "$S" "$shot/disposable.bin"
 cp "$keys" "$shot/scene.keys"
 set +e
 (
 cd "$shot"
 python3 "$D/watch-run.py" env FM1_STRICT=1 FM1_NO_SKIP=1 FM1_BENCH_KEYS="@$shot/scene.keys" FM1_USB_SEND="$USB" FM1_USB_TRACE=1 \
   FM1_DNBOX_PCM="$shot/audio.s16le" FM1_DUMPRAM="$shot/ram.bin" "$R/restored-gate/fm1-live-ba83317a" \
   --fwsc "$R/out/DnBOX-0.0.27-SMK37-$board-73749f55-test.fwsc" --mhz 360 --usb --shot 8000 --lcd-only --flash "$shot/disposable.bin"
 )
 status=$?
 set -e
 cp "$shot/run.log" "$D/elite-$q.transport.log"
 python3 "$D/fault.py" "$E" "$shot" "$q" read | tee "$D/elite-$q.fault.json"
 [[ $status == 0 ]]
 cmp "$S" "$shot/disposable.bin"
 cmp "$keys" "$shot/scene.keys"
 sha256sum "$shot"/{disposable.bin,scene.keys,audio.s16le}
 node "$H/owner-scan.mjs" receipt "$shot/run.log" >"$shot/owner.json"
 python3 "$H/probe.py" "$E" "$shot" | tee "$D/elite-$q.result.json"
 cp "$shot/owner.json" "$D/elite-$q.owner.json"
 rm -r "$shot"; rm "$E" "$S"
done
flock -u 9
