#!/usr/bin/env bash
set -euo pipefail
packet=$(cd -- "$(dirname -- "$0")" && pwd)
if [[ $# != 2 ]]; then
  printf 'Usage: %s EXACT_CHECKPOINT SOURCE_AUTHENTICATED_S1_PACK\n' "$0" >&2
  exit 2
fi
donor=$(realpath "$1")
pack=$(realpath "$2")
test -s "$pack"
(cd "$packet" && sha256sum -c source-patch.sha256)
(cd "$donor" && sha256sum -c "$packet/PREIMAGE.sha256")
work=$(mktemp -d "${TMPDIR:-/tmp}/dn-s1-syncjoin-replay.XXXXXX")
printf 'Private host replay directory %s\n' "$work"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
for side in base join; do
  mkdir -p "$work/$side/build"
  for path in firmware assets experiments tools tests host ble; do
    cp -aL "$donor/$path" "$work/$side/"
  done
  cp -aL "$donor/build/gen" "$work/$side/build/"
  cp "$packet/ui_synthone_sync_storage_test.c" "$work/$side/tests/host/"
done
git -C "$work/join" apply --check "$packet/synthone-pwm2-sync-owner.patch"
git -C "$work/join" apply "$packet/synthone-pwm2-sync-owner.patch"
(cd "$work/join" && sha256sum -c "$packet/POSTIMAGE.sha256")
cp "$packet/ui_synthone_oscillator_editor_test.c" "$work/join/tests/host/"

run() {
  (cd "$work/$1" && SMC_UI_TEST_FILE="tests/host/$2.c" SMC_UI_OUT="$work/$1-$2" \
    bash tests/host/sj6-smc-ui.sh) > "$work/$1-$2.log" 2>&1
  grep 'PASS' "$work/$1-$2.log"
}
run join ui_synthone_sync_storage_test
S1_JOIN_OBJECTS="$work/join-ui_synthone_sync_storage_test" run base ui_synthone_sync_storage_test
run base ui_superj6_legacy_pcm_test
run join ui_superj6_legacy_pcm_test
cmp "$work/"{base,join}-ui_superj6_legacy_pcm_test/legacy.pcm
sha256sum "$work/"{base,join}-ui_superj6_legacy_pcm_test/legacy.pcm
run join ui_synthone_oscillator_editor_test
S1_EDITOR_TEST_PACK="$pack" "$work/join-ui_synthone_oscillator_editor_test/test" \
  > "$work/factory-editor.log" 2>&1
grep 'PASS' "$work/factory-editor.log"
for test in ui_superj6_sync_test ui_superj6_pwm1_lfo2_test ui_synthone_filter_link_test ui_arp_engine_test ui_arp_keyboard_test; do
  run join "$test"
done
for side in base join; do
  (cd "$work/$side" && "${CC:-cc}" -std=c11 -O2 -fno-fast-math -ffp-contract=off -ffunction-sections -fdata-sections \
    -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp \
    "$packet/factory-probe.c" firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c \
    firmware/src/dsp/s1pack/s1pack.c -Wl,--gc-sections -lm -o "$work/factory-$side")
  "$work/factory-$side" "$pack" "$work/factory-$side.pcm"
done
cmp "$work/factory-base.pcm" "$work/factory-join.pcm"
sha256sum "$work"/factory-*.pcm
