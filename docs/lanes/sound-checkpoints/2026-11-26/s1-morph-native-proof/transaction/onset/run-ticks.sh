#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd); R=$(dirname "$D")
T=$(mktemp -d /tmp/dnbox-emulator-only-XXXXXXXX)
trap 'rm -rf "$T"' EXIT
exec 9>/tmp/dnbox-gate-firmware.lock
for arm in ${ARMS:-control stack}; do
 aux="$R/s1-morph-native-909txn-tick-$arm"
 read -r EH PH <"$aux/pins.txt"
 cp "$R/s1-morph-native-909txn/replay-overlay.py" "$aux"
 sed -i "s/e49d0701606ba8671839cf3887a7b52de24769b308995a35c6c82091ae973e23/$EH/;s/03404219da834ca9ecf017d51dcd0d95d76b16bc2179b1778035d0079c5427ba/$PH/" "$aux/replay-overlay.py"
 mkdir -p "$D/tick-$arm"
 for m in ${MODELS:-elite pro}; do
  h="$EH"; [[ $m == elite ]] || h="$PH"
  elf="$aux/tree/widen-native/$m/never-install.elf"
  seed="$T/$arm-$m.bin"; s=$(python3 "$aux/replay-overlay.py" make "$m" "$elf" "$h" "$seed")
  printf '%s  %s\n%s  %s\n' "$h" "$elf" "$s" "$seed" | sha256sum --status -c -
  python3 "$aux/replay-overlay.py" check "$m" "$elf" "$h" "$seed"
  d="$D/tick-$arm/$m"; mkdir "$d" "$d/work"
  cp "$seed" "$d/disposable.bin"
  (
  cd "$d"
  flock -x 9
  FM1_STRICT=1 FM1_NO_SKIP=1 FM1_DNBOX_PCM="$d/audio.s16le" FM1_DUMPRAM="$d/ram.bin" \
      "$R/restored-gate/fm1-live-ba83317a" --fwsc "$R/out/DnBOX-0.0.27-SMK37-$m-73749f55-test.fwsc" \
      --mhz 360 --shot 2850 --flash "$d/disposable.bin" >run.log 2>&1
  flock -u 9
  )
  cmp "$seed" "$d/disposable.bin"
  python3 "$D/read-tick.py" "$elf" "$d/ram.bin" "$d/audio.s16le" >"$d/receipt.json"
 done
done
