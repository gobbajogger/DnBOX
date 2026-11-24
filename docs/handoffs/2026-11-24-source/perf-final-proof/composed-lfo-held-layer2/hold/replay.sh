#!/bin/bash
# Local host-only isolated replay. No emulator, devices, network or app writes.
set -euo pipefail
[ "$#" -eq 4 ] || { echo "Usage: bash replay.sh HP_SOURCE WIDEN_TUNE_SOURCE FULL_SUPPORT_SOURCE AUTHORIZED_BANK_ZIP"; exit 2; }
root=$(cd -- "$(dirname -- "$0")" && pwd)
hp=$(realpath "$1"); widen=$(realpath "$2"); full=$(realpath "$3"); bank=$(realpath "$4")
w=$(mktemp -d "${TMPDIR:-/tmp}/sj6-held-replay.XXXXXX")
printf 'Host-only work and logs: %s\n' "$w"
for arm in old hp widen; do
  mkdir "$w/$arm"
  if [[ $arm == widen ]]; then src=$widen; else src=$hp; fi
  cp -aL "$src"/{firmware,tests,host,tools,assets,experiments,ble} "$w/$arm/"
  mkdir "$w/$arm/build"; cp -aL "$src/build/gen" "$w/$arm/build/"
done
(cd "$w/hp"; patch --no-backup-if-mismatch -p1 <"$root/held-exact-hp.patch"; sha256sum --quiet -c "$root/site-POSTIMAGE.sha256")
(cd "$w/widen"; patch --no-backup-if-mismatch -p1 <"$root/held-composed-widen-tune.patch"; sha256sum --quiet -c "$root/widen-tune-POSTIMAGE.sha256")
cp "$root/ui_superj6_held_old_reader_test.c" "$w/old/tests/host/"
mkdir "$w/fixtures" "$w/overlap"
python3 "$root/generate-authored-shape-evidence.py" "$full" "$bank" "$w/fixtures"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1
defs='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
profile() {
 local arm=$1 name=$2 target=$3 flags=${4:-}
 (cd "$w/$arm"; PYTHONPATH="$full/tools:$PWD/tools" SMC_UI_TEST_FILE="tests/host/$target.c"  SMC_UI_OUT="$w/$name" X0X_HOST_DEFS="$defs $flags" sh tests/host/sj6-smc-ui.sh) >"$w/$name.log" 2>&1
}
(cd "$w/hp"; cc -std=c99 -O2 -Wall -Wextra -Werror -DX0X_SUPERJ6_BUILTIN_INIT=1   -fsanitize=undefined -Ifirmware/src tests/host/sj6_lfo2_held_entropy_test.c -o "$w/entropy"; "$w/entropy")
SJ6_SHAPE_CASE=or profile hp shapes ui_superj6_lfo2_shape_test -Wl,--wrap=dnbx_pwm_from_unit
(cd "$w/hp"; for route in 00 01 10 11; do SJ6_SHAPE_CASE=$route "$w/shapes/test"; done)
profile hp bank ui_superj6_vibrato_authored_test "-Wl,--wrap=juno6_init -I$w/fixtures -I$full/experiments/sound-hardware-v1/sj6-factory"
profile old old-reader ui_superj6_held_old_reader_test
sed '/^int main(void)/,$d' "$w/widen/tests/host/ui_superj6_layer1_tuning_test.c" > "$w/overlap/ui_superj6_tuning_helpers.inc"
profile widen overlap ui_superj6_hold_tuning_overlap_test "-Wl,--wrap=juno6_init -I$w/widen/tests/host"
(cd "$w/widen"; SJ6_OWNER=1 SMC_UI_SHOTS="$w/overlap" "$w/overlap/test")
for f in "$w"/{shapes,bank,old-reader,overlap}.log; do grep -E '^PASS|^2048|^640|^owner|^SuperJ6' "$f"; done
