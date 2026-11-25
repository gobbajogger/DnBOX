#!/usr/bin/env bash
set -euo pipefail
src="$(realpath "$1")/firmware/src/dsp"
p=$(cd "$(dirname "$0")" && pwd)
(cd "$src"; sha256sum -c "$p/../sj6-mod-wheel-vibrato/PREIMAGE.sha256")
w=$(mktemp -d "${TMPDIR:-/tmp}/sj6-mod-replay.XXXXXX")
trap 'rm -rf "$w"' EXIT
mkdir -p "$w/firmware/src/dsp"
next="$w/firmware/src/dsp"
cp "$src"/{juno6.c,juno6.h,juno6_factory.inc} "$next/"
build() {
 "${CC:-cc}" -std=c11 -O2 -fno-fast-math -ffp-contract=off -I"$next" -I"$src" -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 "$p/test.c" "$next/juno6.c" "$src/dnbx_core.c" -lm -o "$w/test"
}
(cd "$w";patch --fuzz=0 -p1 <"$p/../sj6-mod-wheel-vibrato/vibrato-destination.patch")
(cd "$w";patch --fuzz=0 -p1 <"$p/../sj6-mod-wheel-lfo2/lfo2-destination.patch")
(cd "$next";sha256sum -c "$p/PREIMAGE.sha256")
build
"$w/test" "$w/before.pcm" 0
if "$w/test" "$w/red.pcm" 1 >"$w/red.log" 2>&1; then exit 1;fi
grep -F 'FAIL 24: juno6_set_mod_destination(s,5)' "$w/red.log"
(cd "$w";patch --fuzz=0 -p1 <"$p/cutoff-destination.patch")
(cd "$next";sha256sum -c "$p/POSTIMAGE.sha256")
build
"$w/test" "$w/after.pcm" 1
cmp "$w/before.pcm" "$w/after.pcm"
sha256sum "$w/after.pcm"
