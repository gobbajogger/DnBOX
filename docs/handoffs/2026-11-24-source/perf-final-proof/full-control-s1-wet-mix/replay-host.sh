#!/usr/bin/env bash
# Build and run the same isolated host profiles. No device or native targets.
set -euo pipefail
packet=$(cd -- "$(dirname -- "$0")" && pwd)
if [[ $# != 2 ]]; then
    printf 'Usage: %s PRODUCT_PREIMAGE AUTHENTICATED_S1_PACK\n' "$0" >&2
    exit 2
fi
source=$(realpath "$1")
export S1_EDITOR_TEST_PACK=$(realpath "$2")
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
(cd "$packet" && sha256sum -c SHA256SUMS >/dev/null)
printf '%s  %s\n' "$(cat "$packet/S1-PACK.sha256")" "$S1_EDITOR_TEST_PACK" | sha256sum -c --status
work=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-s1-wet-host.XXXXXX")
printf 'Private host replay %s\n' "$work"
mkdir -p "$work/base/build"
for name in firmware assets experiments tools tests host ble; do
    cp -aL "$source/$name" "$work/base/"
done
cp -aL "$source/build/gen" "$work/base/build/"
manifest() {
    (cd "$work/$1" && find firmware -type f -print0 | sort -z | xargs -0 sha256sum)
}
manifest base | cmp - "$packet/PRODUCT-PREIMAGE.sha256"
cp -a "$work/base" "$work/join"
git -C "$work/join" apply --check "$packet/s1-wet-mix-full-control.patch"
git -C "$work/join" apply "$packet/s1-wet-mix-full-control.patch"
manifest join | cmp - "$packet/PRODUCT-POSTIMAGE.sha256"
printf 'Exact preimage and postimage verified for %s firmware files\n' "$(wc -l < "$packet/PRODUCT-PREIMAGE.sha256")"

profile() {
    local tree=$1 fixture=$2 evidence=$3
    (cd "$work/$tree" && SMC_UI_TEST_FILE="tests/host/$fixture.c" SMC_UI_OUT="$work/$evidence" \
       bash tests/host/sj6-smc-ui.sh) > "$work/$evidence.log" 2>&1
}
instrumented() {
    local tree=$1 base=$2 kind=$3
    cp -a "$work/$base" "$work/$tree"
    (
        cd "$work/$tree"
        sed '/^int main(void)$/,$d' tests/host/ui_synthone_filter_mode_test.c > tests/host/lfo-test-helpers.h
        cp "$packet"/ui_synthone_{wet_mix,lfo,lfo_clock}_test.c tests/host/
        python3 "$packet/instrument-host-$kind.py"
    )
}
instrumented wet-base base wet
instrumented wet-join join wet
if X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_WET_RED=1" profile wet-base ui_synthone_wet_mix_test red; then
    printf 'Expected old product to lack MIX editing\n' >&2
    exit 1
fi
grep 'FAIL.*engine_s1_patch_read(0, DELAY_MIX, &absent)' "$work/red.log"
profile wet-join ui_synthone_wet_mix_test positive
grep -E '^(owner|PASS)' "$work/positive.log"
X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_WET_LEGACY=1" S1_FILTER_PROJECTS="$work/positive" \
    profile wet-base ui_synthone_wet_mix_test old-reader
grep '^PASS' "$work/old-reader.log"

for tree in base join; do
    host="$work/$tree/tests/host"
    sed '/^int main(void)$/,$d' "$host/ui_superj6_vibrato_mixed_test.c" > "$host/vibrato-mixed-helpers.h"
    sed -n '/^enum { PROJECT_EXPORT, PROJECT_MOUNT, PROJECT_VERIFY };/,/^static s1_patch_value read_slot/p' \
        "$host/ui_synthone_filter_mode_test.c" | sed '$d' > "$host/lfo-objects-helpers.h"
    cp "$packet/ui_s1_wet_shared_test.c" "$host/"
done
profile join ui_s1_wet_shared_test shared
grep '^PASS' "$work/shared.log"
X0X_HOST_DEFS="$X0X_HOST_DEFS -DS1_WET_MIXED_LEGACY=1" S1_FILTER_PROJECTS="$work/shared" \
    profile base ui_s1_wet_shared_test old-shared
grep '^PASS' "$work/old-shared.log"

instrumented rate join rate
profile rate ui_synthone_lfo_test rate
grep '^PASS' "$work/rate.log"
instrumented clock join clock
S1_LFO_CAPTURE_INIT=1 profile clock ui_synthone_lfo_clock_test clock
grep '^PASS' "$work/clock.log"
cp -a "$work/clock" "$work/broken"
sed -i '/    s1c_set(e, P_PAN_RATE, part->clock_factory_pan);/d; /    s1c_set(e, P_DELAY_TIME, part->clock_factory_delay);/d' \
    "$work/broken/firmware/src/app/s1c_part.c"
if S1_LFO_ROLLBACK_WITNESS=1 profile broken ui_synthone_lfo_clock_test broken; then
    printf 'Expected clock rollback mutant to fail\n' >&2
    exit 1
fi
grep -F 'targets[clock_ids[i]] == expected[locked][i]' "$work/broken.log"

compact=(-O2 -fno-fast-math -ffp-contract=off -ffunction-sections -fdata-sections
    -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -Ifirmware/src -Ifirmware/src/app
    -Ifirmware/src/dsp -Ifirmware/src/dsp/s1c)
for side in base join; do
    (cd "$work/$side" && "${CC:-cc}" -std=c11 "${compact[@]}" "$packet/factory-probe.c" firmware/src/app/s1c_part.c \
       firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c \
       -Wl,--gc-sections -lm -o "$work/$side-factory")
    "$work/$side-factory" "$S1_EDITOR_TEST_PACK" "$work/$side-factory.pcm"
done
cmp "$work/base-factory.pcm" "$work/join-factory.pcm"
(cd "$work" && sha256sum {base,join}-factory.pcm)
"${CC:-cc}" -std=c11 -O2 -fsanitize=undefined -fno-sanitize-recover=all \
    "$packet/s1-editor-15b-oracle.c" -o "$work/oracle"
"$work/oracle"

