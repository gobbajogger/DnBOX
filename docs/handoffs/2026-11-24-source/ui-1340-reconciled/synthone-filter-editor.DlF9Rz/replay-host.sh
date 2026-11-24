#!/usr/bin/env bash
set -euo pipefail
packet=$(cd -- "$(dirname -- "$0")" && pwd)
prior="$packet/../synthone-sync-join.s979TF"
if [[ $# != 2 ]]; then
    printf 'Usage: %s EXACT_CHECKPOINT AUTHENTICATED_S1_PACK\n' "$0" >&2
    exit 2
fi
donor=$(realpath "$1")
pack=$(realpath "$2")
test -s "$pack"
(cd "$packet" && sha256sum -c source-patch.sha256)
work=$(mktemp -d "${TMPDIR:-/tmp}/dn-s1-filter-replay.XXXXXX")
printf 'Isolated host replay %s\n' "$work"
mkdir -p "$work/base/build"
for path in firmware assets experiments tools tests host ble; do
    cp -aL "$donor/$path" "$work/base/"
done
cp -aL "$donor/build/gen" "$work/base/build/"
if ! (cd "$work/base" && sha256sum -c "$packet/PREIMAGE.sha256" >/dev/null 2>&1); then
    (cd "$prior" && sha256sum -c source-patch.sha256)
    (cd "$work/base" && sha256sum -c "$prior/PREIMAGE.sha256")
    git -C "$work/base" apply "$prior/synthone-pwm2-sync-owner.patch"
fi
(cd "$work/base" && sha256sum -c "$packet/PREIMAGE.sha256")
cp -a "$work/base" "$work/join"
git -C "$work/join" apply --check "$packet/synthone-filter-mode-track.patch"
git -C "$work/join" apply "$packet/synthone-filter-mode-track.patch"
(cd "$work/join" && sha256sum -c "$packet/POSTIMAGE.sha256")

export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
for side in base join; do
    cp "$packet/ui_synthone_filter_mode_test.c" "$work/$side/tests/host/"
done
cp "$prior"/ui_synthone_oscillator_editor_test.c "$prior"/ui_synthone_sync_storage_test.c "$work/join/tests/host/"

run_ui() {
    (cd "$work/$1" && SMC_UI_TEST_FILE="tests/host/$2.c" SMC_UI_OUT="$work/$1-$2" \
        bash tests/host/sj6-smc-ui.sh) > "$work/$1-$2.log" 2>&1
    grep 'PASS' "$work/$1-$2.log"
}
compile_compact() {
    (cd "$work/$1" && "${CC:-cc}" -std=c11 -O2 -fno-fast-math -ffp-contract=off -ffunction-sections -fdata-sections \
        -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp \
        "$packet/$2.c" firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c \
        firmware/src/dsp/s1pack/s1pack.c -Wl,--gc-sections -lm -o "$work/$1-$2")
}

run_ui join ui_synthone_filter_mode_test
S1_FILTER_PROJECTS="$work/join-ui_synthone_filter_mode_test" \
    X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_FILTER_LEGACY=1" run_ui base ui_synthone_filter_mode_test
for test in filter-coefficients filter-factory-values; do
    compile_compact join "$test"
    "$work/join-$test" "$pack"
done
for side in base join; do
    compile_compact "$side" factory-probe
    "$work/$side-factory-probe" "$pack" "$work/$side-factory.pcm"
    run_ui "$side" ui_superj6_legacy_pcm_test
done
cmp "$work/"{base,join}-factory.pcm
sha256sum "$work/"{base,join}-factory.pcm
cmp "$work/"{base,join}-ui_superj6_legacy_pcm_test/legacy.pcm
sha256sum "$work/"{base,join}-ui_superj6_legacy_pcm_test/legacy.pcm
run_ui join ui_synthone_oscillator_editor_test
S1_EDITOR_TEST_PACK="$pack" "$work/join-ui_synthone_oscillator_editor_test/test" > "$work/factory-editor.log"
grep 'PASS' "$work/factory-editor.log"
for test in ui_synthone_filter_link_test ui_synthone_sync_storage_test ui_superj6_sync_test ui_superj6_pwm1_lfo2_test ui_arp_engine_test ui_arp_keyboard_test; do
    run_ui join "$test"
done
