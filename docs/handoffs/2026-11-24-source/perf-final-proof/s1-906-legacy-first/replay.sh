#!/usr/bin/env bash
set -euo pipefail
umask 077
q=$(realpath "$(dirname "$0")")
[[ $# == 4 ]] || { echo 'Usage: replay.sh EXACT_SOURCE UPLOAD_ZIP COMPRESSION_LAB_ZIP LEGACY_S1_PACK' >&2; exit 2; }
src=$(realpath "$1"); upload=$(realpath "$2"); lab=$(realpath "$3"); old=$(realpath "$4")
w=$(mktemp -d "${TMPDIR:-/tmp}/s1-legacy-replay.XXXXXX")
trap 'rm -rf -- "$w"' EXIT
(
 cd "$src"
 got=$(find firmware -type f -print0 | sort -z | xargs -0 sha256sum | sha256sum | awk '{print $1}')
 [[ "$got" == 'f7d17a572dc1affdde651cb8764857da92ddb6308a3194d2bb72618ecbf98c12' ]]
)
python3 "$q/build_private.py" "$src" "$upload" "$lab" "$old" "$w"
(
 cd "$src"
 "${CC:-cc}" -std=c11 -O2 -fno-fast-math -ffp-contract=off -ffunction-sections -fdata-sections \
  -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ifirmware/src/dsp/s1c \
  "$q/render_verified.c" firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c \
  firmware/src/dsp/s1pack/s1pack.c -Wl,--gc-sections -lm -o "$w/render"
)
"$w/render" "$old" "$w/combined.pack" "$w/check.bin"
mkdir -p "$w/src/build"
for p in assets firmware experiments tests tools host ble; do cp -aL "$src/$p" "$w/src/"; done
cp -aL "$src/build/gen" "$w/src/build/"
(
 cd "$w/src"
 sed '/^int main(void)$/,$d' tests/host/ui_synthone_filter_mode_test.c > tests/host/lfo-test-helpers.h
 cp "$q/reopen_old_project.c" tests/host/
 OLD_PACK="$old" COMBINED_PACK="$w/combined.pack" OLD_ALIAS_PAIR="$w/aliases.bin" X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 \
  X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1' \
  SMC_UI_OUT="$w/reopen" SMC_UI_TEST_FILE=tests/host/reopen_old_project.c sh tests/host/sj6-smc-ui.sh
) > "$w/reopen.log" 2>&1 || { grep -E '(^FAIL |error:)' "$w/reopen.log" >&2 || :; exit 1; }
grep '^PASS ' "$w/reopen.log"
