#!/usr/bin/env bash
# Private host-only replay: no device, native, or external writes.
set -euo pipefail
packet=$(realpath "$(dirname "$0")")
[[ $# == 2 ]] || { echo "usage: $0 EXACT_PRODUCT_SOURCE AUTHENTICATED_S1_PACK" >&2; exit 2; }
source=$(realpath "$1")
export S1_EDITOR_TEST_PACK=$(realpath "$2")
printf '%s  %s\n' "$(cat "$packet/S1-PACK.sha256")" "$S1_EDITOR_TEST_PACK" | sha256sum -c --status
(cd "$packet"; sha256sum -c SHA256SUMS --status)
(cd "$source"; sha256sum -c "$packet/COMPOSED-HP-PREIMAGE.sha256" --status)
work=$(mktemp -d "${TMPDIR:-/tmp}/s1-widen-host.XXXXXX")
printf 'Private host evidence: %s\n' "$work"
mkdir -p "$work/base/build"
for path in firmware assets experiments tests tools host ble; do cp -aL "$source/$path" "$work/base/"; done
cp -aL "$source/build/gen" "$work/base/build/"
cp -a "$work/base" "$work/join"
(cd "$work/base"; sha256sum -c "$packet/PREIMAGE.sha256" --status)
(cd "$work/join"; git apply "$packet/widen.patch"; sha256sum -c "$packet/POSTIMAGE.sha256" --status)
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
compact=(-O2 -fno-fast-math -ffp-contract=off -ffunction-sections -fdata-sections
   -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ifirmware/src/dsp/s1c)
for side in base join; do
  (
    cd "$work/$side"
    "${CC:-cc}" -std=c11 "${compact[@]}" "$packet/factory-probe.c" firmware/src/app/s1c_part.c \
        firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -Wl,--gc-sections -lm -o "$work/$side-factory"
    "$work/$side-factory" "$S1_EDITOR_TEST_PACK" "$work/$side.pcm"
    sed '/^int main(void)$/,$d' tests/host/ui_synthone_filter_mode_test.c > tests/host/lfo-test-helpers.h
    cp "$packet"/ui_synthone_{widen,reverb_hp,wet_mix}_test.c "$packet/ui_s1_widen_lcd_test.c" tests/host/
    python3 "$packet/instrument-host-wet.py"
  )
done
cmp "$work/base.pcm" "$work/join.pcm"
sha256sum "$work"/{base,join}.pcm
profile() {
  (cd "$work/$1"; SMC_UI_OUT="$work/$2" SMC_UI_TEST_FILE="tests/host/${3:-ui_synthone_widen_test}.c" sh tests/host/sj6-smc-ui.sh) > "$work/$2.log" 2>&1
}
if X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_WET_RED=1" profile base red; then echo 'expected WIDEN absent on old editor' >&2; exit 1; fi
grep '^FAIL.*engine_s1_patch_read(0,WIDEN,&absent)' "$work/red.log"
profile join positive
S1_FILTER_PROJECTS="$work/positive" X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_WET_LEGACY=1" profile base old
profile join prior-wet ui_synthone_wet_mix_test
profile join prior-hp ui_synthone_reverb_hp_test
grep -E '^(PASS|owner)' "$work"/{positive,old,prior-wet,prior-hp}.log
profile join lcd ui_s1_widen_lcd_test
grep '^PASS' "$work/lcd.log"
