#!/usr/bin/env bash
set -euo pipefail
packet=$(realpath "$(dirname "$0")")
[[ $# == 2 ]] || { printf '%s\n' 'Usage: replay-host.sh EXACT_COARSE_SOURCE AUTHENTICATED_S1_PACK' >&2; exit 2; }
source=$(realpath "$1")
export S1_EDITOR_TEST_PACK=$(realpath "$2")
printf '%s  %s\n' "$(cat "$packet/S1-PACK.sha256")" "$S1_EDITOR_TEST_PACK" | sha256sum -c --status
(cd "$packet"; sha256sum -c SHA256SUMS --status)
(cd "$source"; sha256sum -c "$packet/COARSE-PREIMAGE.sha256" --status)
w=$(mktemp -d "${TMPDIR:-/tmp}/s1-voice-output-coarse-host.XXXXXX")
printf 'Private host output: %s\n' "$w"
mkdir -p "$w/base/build"
for p in firmware assets experiments tests tools host ble; do cp -aL "$source/$p" "$w/base/"; done
cp -aL "$source/build/gen" "$w/base/build/"
cp -a "$w/base" "$w/join"
(cd "$w/join"; git apply "$packet/voice-output.patch"; sha256sum -c "$packet/COARSE-VOICE-OUTPUT-POSTIMAGE.sha256" --status)
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
for side in base join; do
 (
  cd "$w/$side"
  "${CC:-cc}" -std=c11 -O2 -fno-fast-math -ffp-contract=off -ffunction-sections -fdata-sections \
    -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -Ifirmware/src -Ifirmware/src/app \
    -Ifirmware/src/dsp -Ifirmware/src/dsp/s1c "$packet/factory-probe.c" firmware/src/app/s1c_part.c \
    firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -Wl,--gc-sections -lm -o "$w/$side-factory"
  "$w/$side-factory" "$S1_EDITOR_TEST_PACK" "$w/$side.pcm"
  sed '/^int main(void)$/,$d' tests/host/ui_synthone_filter_mode_test.c > tests/host/lfo-test-helpers.h
  cp "$packet"/ui_synthone_{legato,part_level}_test.c "$packet/ui_s1_voice_output_lcd_test.c" tests/host/
  python3 "$packet/instrument-host-wet.py"
  python3 "$packet/instrument-host-legato.py"
 )
done
cmp "$w/base.pcm" "$w/join.pcm"
sha256sum "$w"/{base,join}.pcm
profile() {
 (cd "$w/$1"; SMC_UI_OUT="$w/$2" SMC_UI_TEST_FILE="tests/host/$3.c" sh tests/host/sj6-smc-ui.sh) >"$w/$2.log" 2>&1
}
for field in part_level legato; do
 if X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_WET_RED=1" profile base "$field-red" "ui_synthone_${field}_test"; then echo "Expected old $field to be absent" >&2; exit 1; fi
 grep '^FAIL.*engine_s1_patch_read' "$w/$field-red.log"
 X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_COMBINED_LEVEL=1" profile join "$field-positive" "ui_synthone_${field}_test"
 S1_FILTER_PROJECTS="$w/$field-positive" X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_WET_LEGACY=1" profile base "$field-old" "ui_synthone_${field}_test"
 grep -E '^(PASS|owner|part)' "$w/$field-positive.log" "$w/$field-old.log"
done
profile join lcd ui_s1_voice_output_lcd_test
grep '^PASS' "$w/lcd.log"
