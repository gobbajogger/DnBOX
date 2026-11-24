#!/usr/bin/env bash
set -euo pipefail
packet=$(realpath "$(dirname "$0")")
[[ $# == 2 ]] || { printf '%s\n' 'Usage: replay-host.sh EXACT_OSC_MIX_SOURCE AUTHENTICATED_S1_PACK' >&2; exit 2; }
source=$(realpath "$1")
export S1_EDITOR_TEST_PACK=$(realpath "$2")
printf '%s  %s\n' "$(cat "$packet/S1-PACK.sha256")" "$S1_EDITOR_TEST_PACK" | sha256sum -c --status
(cd "$packet"; sha256sum -c SHA256SUMS --status)
(cd "$source"; sha256sum -c "$packet/PREIMAGE.sha256" --status)
w=$(mktemp -d "${TMPDIR:-/tmp}/s1-resonance-lfo-host.XXXXXX")
printf 'Private host output: %s\n' "$w"
mkdir -p "$w/base/build"
for p in firmware assets experiments tests tools host ble; do cp -aL "$source/$p" "$w/base/"; done
cp -aL "$source/build/gen" "$w/base/build/"
cp -a "$w/base" "$w/join"
(cd "$w/join"; git apply "$packet/resonance-lfo.patch"; sha256sum -c "$packet/POSTIMAGE.sha256" --status)
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
  cp "$packet"/ui_synthone_resonance_lfo_test.c "$packet"/ui_synthone_osc_mix_lfo_test.c "$packet/ui_s1_osc_route_page_reach_test.c" "$packet/ui_s1_resonance_lfo_lcd_test.c" tests/host/
  python3 "$packet/instrument-host-wet.py"
  printf '\nconst s1c *host_s1_owner(unsigned o) { return &original_bass[o].dsp.synthone.e; }\n' >> firmware/src/app/engine.c
 )
done
cmp "$w/base.pcm" "$w/join.pcm"
sha256sum "$w"/{base,join}.pcm
profile() {
 (cd "$w/$1"; SMC_UI_OUT="$w/$2" SMC_UI_TEST_FILE="tests/host/$3.c" sh tests/host/sj6-smc-ui.sh) >"$w/$2.log" 2>&1
}

if X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_WET_RED=1" profile base res-red ui_synthone_resonance_lfo_test; then
  echo 'Expected old depth to be absent' >&2
  exit 1
fi
grep '^FAIL.*engine_s1_patch_read' "$w/res-red.log"
profile join res-positive ui_synthone_resonance_lfo_test
S1_FILTER_PROJECTS="$w/res-positive" X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_WET_LEGACY=1" profile base res-old ui_synthone_resonance_lfo_test
grep -E '^(PASS|owner)' "$w"/res-{positive,old}.log
X0X_HOST_DEFS="$X0X_HOST_DEFS -DOSC_ROUTE_LAYOUT=1" profile base pages-base ui_s1_osc_route_page_reach_test
X0X_HOST_DEFS="$X0X_HOST_DEFS -DOSC_ROUTE_LAYOUT=1" profile join pages-join ui_s1_osc_route_page_reach_test
for f in "$w/pages-base"/{A,B}-*.png; do cmp "$f" "$w/pages-join/${f##*/}"; done
grep '^PASS' "$w"/pages-{base,join}.log
echo "PASS 14 unchanged SuperJ6 actual LCDs byte-exact"
profile join lcd ui_s1_resonance_lfo_lcd_test
grep "^PASS" "$w/lcd.log"
profile join retained-osc ui_synthone_osc_mix_lfo_test
grep "^PASS" "$w/retained-osc.log"
