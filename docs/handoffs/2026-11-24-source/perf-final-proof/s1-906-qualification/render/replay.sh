#!/usr/bin/env bash
set -euo pipefail
umask 077
q=$(realpath "$(dirname "$0")")
[[ $# == 4 ]] || { echo 'Usage: replay.sh EXACT_SOURCE UPLOADED_ARCHIVE COMPRESSION_LAB_ARCHIVE LEGACY_S1_PACK' >&2; exit 2; }
src=$(realpath "$1"); upload=$(realpath "$2"); lab=$(realpath "$3"); old=$(realpath "$4")
w=$(mktemp -d "${TMPDIR:-/tmp}/s1-private-corpus.XXXXXX")
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
"$w/render" "$old" "$w/union.pack" "$w/check.bin"
