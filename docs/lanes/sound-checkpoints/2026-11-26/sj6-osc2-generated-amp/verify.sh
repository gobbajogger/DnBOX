#!/bin/bash
set -euo pipefail
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
donor=${1:?pass exact dnbx_core.c from the receipt}
headers=${2:?pass exact include directory from the receipt}
out=$(mktemp -d "${TMPDIR:-/tmp}/sj6-generated-verify.XXXXXXXX")
trap 'rm -rf "$out"' EXIT
python3 "$here/apply_candidate.py" "$donor" "$out/candidate.c"
flags=(-std=c99 -O2 -ffp-contract=off -DX0X_HOST -DX0X_SJ6_MONO32_PROBE=1 -DX0X_SJ6_OTA_NATIVE=1 -DDNBX_HOST_MAIN_SOURCE_ORACLE=1 -I "$headers")
# Compile from stdin in each case so input filename isn't part of the comparison.
cc "${flags[@]}" -DDNBX_HOST_SJ6_CURRENT_OSC2_GENERATED_AMP=0 -x c -c - -o "$out/donor-off.o" < "$donor"
cc "${flags[@]}" -DDNBX_HOST_SJ6_CURRENT_OSC2_GENERATED_AMP=0 -x c -c - -o "$out/patch-off.o" < "$out/candidate.c"
cc "${flags[@]}" -DDNBX_HOST_SJ6_CURRENT_OSC2_GENERATED_AMP=1 -x c -c - -o "$out/patch-on.o" < "$out/candidate.c"
cmp "$out/donor-off.o" "$out/patch-off.o"
if cmp -s "$out/patch-on.o" "$out/patch-off.o"; then echo 'Enabled path did not build' >&2; exit 1; fi
(cd "$out"; shasum -a 256 donor-off.o patch-off.o patch-on.o)
echo 'PASS default-disabled compiler output exact; optional generated OSC2 proof compiles'
