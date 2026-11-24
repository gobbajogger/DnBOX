#!/usr/bin/env bash
set -euo pipefail
packet=$(cd -- "$(dirname -- "$0")" && pwd)
if [[ $# != 2 && $# != 5 ]]; then
    printf 'Usage: %s PRODUCT_PREIMAGE S1_PACK [VIB_FIXTURES VIB_READER_SOURCE SAGA_FIXTURES]\n' "$0" >&2
    exit 2
fi
source=$(realpath "$1")
export S1_EDITOR_TEST_PACK=$(realpath "$2")
if [[ $# == 5 ]]; then
    vib=$(realpath "$3")
    reader=$(realpath "$4")
    saga=$(realpath "$5")
fi
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
work=$(mktemp -d "${TMPDIR:-/tmp}/dn-s1-lfo-full-host.XXXXXX")
printf 'Private host replay %s\n' "$work"
(cd "$packet" && sha256sum -c source-patch.sha256)
mkdir -p "$work/base/build"
for p in firmware assets experiments tools tests host ble; do cp -aL "$source/$p" "$work/base/"; done
cp -aL "$source/build/gen" "$work/base/build/"
(cd "$work/base" && find firmware -type f -print0 | sort -z | xargs -0 sha256sum) \
  | cmp - "$packet/PRODUCT-PREIMAGE.sha256"
(cd "$work/base" && sha256sum -c "$packet/PATCH-PREIMAGE.sha256" >/dev/null)
cp -a "$work/base" "$work/join"
git -C "$work/join" apply --check "$packet/s1-lfo-full-control.patch"
git -C "$work/join" apply "$packet/s1-lfo-full-control.patch"
(cd "$work/join" && sha256sum -c "$packet/PATCH-POSTIMAGE.sha256" >/dev/null)
(cd "$work/join" && find firmware -type f -print0 | sort -z | xargs -0 sha256sum) \
  | cmp - "$packet/PRODUCT-POSTIMAGE.sha256"
printf 'Exact preimage and postimage verified over 313 firmware files\n'

run_ui() {
    (cd "$work/$1" && SMC_UI_TEST_FILE="tests/host/$2.c" SMC_UI_OUT="$work/$3" \
        bash tests/host/sj6-smc-ui.sh) > "$work/$3.log" 2>&1
    tail -4 "$work/$3.log"
}
instrumented() {
    cp -a "$work/$1" "$work/$2"
    sed '/^int main(void)$/,$d' "$work/$2/tests/host/ui_synthone_filter_mode_test.c" \
        > "$work/$2/tests/host/lfo-test-helpers.h"
    cp "$packet/ui_synthone_lfo_test.c" "$packet/ui_synthone_lfo_clock_test.c" "$work/$2/tests/host/"
    (cd "$work/$2" && python3 "$packet/instrument-host-$3.py")
}
for side in base join; do
    host="$work/$side/tests/host"
    sed '/^int main(void)$/,$d' "$host/ui_superj6_vibrato_mixed_test.c" > "$host/vibrato-mixed-helpers.h"
    sed -n '/^enum { PROJECT_EXPORT, PROJECT_MOUNT, PROJECT_VERIFY };/,/^static s1_patch_value read_slot/p' \
        "$host/ui_synthone_filter_mode_test.c" | sed '$d' > "$host/lfo-objects-helpers.h"
    cp "$packet/ui_s1_lfo_shared_full_control_test.c" "$host/"
done
run_ui join ui_s1_lfo_shared_full_control_test mixed
S1_FILTER_PROJECTS="$work/mixed" X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_JOINED_LEGACY=1" \
    run_ui base ui_s1_lfo_shared_full_control_test old-mixed-reader
instrumented join rate rate
run_ui rate ui_synthone_lfo_test observe-rate
instrumented base old-rate rate
S1_FILTER_PROJECTS="$work/observe-rate" X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_LFO_LEGACY=1" \
    run_ui old-rate ui_synthone_lfo_test old-rate-reader
instrumented join clock clock
S1_LFO_CAPTURE_INIT=1 run_ui clock ui_synthone_lfo_clock_test observe-clock
mkdir -p "$work/observe-witness"
S1_LFO_ROLLBACK_WITNESS=1 SMC_UI_SHOTS="$work/observe-witness" "$work/observe-clock/test"
instrumented base old-clock clock
S1_FILTER_PROJECTS="$work/observe-clock" X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_CLOCK_LEGACY=1" \
    run_ui old-clock ui_synthone_lfo_clock_test old-clock-reader
cp -a "$work/clock" "$work/clock-broken"
sed -i '/    s1c_set(e, P_PAN_RATE, part->clock_factory_pan);/d; /    s1c_set(e, P_DELAY_TIME, part->clock_factory_delay);/d' \
    "$work/clock-broken/firmware/src/app/s1c_part.c"
if (cd "$work/clock-broken" && S1_LFO_ROLLBACK_WITNESS=1 SMC_UI_OUT="$work/broken" \
    SMC_UI_TEST_FILE=tests/host/ui_synthone_lfo_clock_test.c bash tests/host/sj6-smc-ui.sh) > "$work/broken.log" 2>&1; then
    printf 'Expected rollback mutant to fail\n' >&2
    exit 1
fi
grep 'lock 0 factory ID 36' "$work/broken.log"
grep 'FAIL.*targets\[clock_ids\[i\]\] == expected\[locked\]\[i\]' "$work/broken.log"

compact=(-O2 -fno-fast-math -ffp-contract=off -ffunction-sections -fdata-sections
    -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ifirmware/src/dsp/s1c)
for side in base join; do
    (cd "$work/$side" && cc -std=c11 "${compact[@]}" "$packet/factory-probe.c" firmware/src/app/s1c_part.c \
       firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c \
       -Wl,--gc-sections -lm -o "$work/$side-factory")
    "$work/$side-factory" "$S1_EDITOR_TEST_PACK" "$work/$side-factory.pcm"
done
cmp "$work/base-factory.pcm" "$work/join-factory.pcm"
sha256sum "$work/"{base,join}-factory.pcm
(
    cd "$work/join"
    for unit in s1c s1u s1c_front; do
        cc -std=c11 "${compact[@]}" -c "firmware/src/dsp/s1c/$unit.c" -o "$work/law-$unit.o"
    done
    c++ -std=c++17 "${compact[@]}" -Ifirmware/src/dsp/synthone/controls/frontend -Wl,--gc-sections \
        "$packet/lfo-rate-law.cpp" "$work"/law-{s1c,s1u,s1c_front}.o -lm -o "$work/lfo-rate-law"
    "$work/lfo-rate-law"
)
for fixture in ui_superj6_vibrato_mixed_test ui_synthone_filter_mode_test ui_synthone_sync_storage_test \
               ui_superj6_sync_test ui_superj6_pwm1_lfo2_test ui_arp_engine_test ui_arp_keyboard_test; do
    run_ui join "$fixture" "$fixture"
done
(
 unset S1_EDITOR_TEST_PACK
 run_ui join ui_synthone_oscillator_editor_test oscillator-init
)
"$work/oscillator-init/test"
if [[ $# == 5 ]]; then
    for f in "$vib/authored_vibrato.inc" "$vib/bank_data.inc" "$saga/saga_toggles.inc"; do
        actual=$(sha256sum "$f" | awk '{print $1}')
        test "$actual" = "$(awk -v name="/${f##*/}" 'index($2,name) {print $1}' "$packet/original-host-inputs.sha256")"
    done
    X0X_HOST_DEFS="$X0X_HOST_DEFS -DSJ6_SAGA_CORPUS -I$saga" run_ui join ui_superj6_authored_saga_test authored-saga
    X0X_HOST_DEFS="$X0X_HOST_DEFS -I$vib -I$reader -Wl,--wrap=juno6_init" \
        run_ui join ui_superj6_vibrato_authored_test authored-vibrato
fi
