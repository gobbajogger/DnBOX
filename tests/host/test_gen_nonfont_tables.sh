#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# When Pillow is missing, run_tests.sh calls gen_nonfont instead of host-build.
# Samples can exist without x0x_drum_tables.h; a custom X0X_BREAK_BANK skips
# gen_builtin_break.sh, so gen_nonfont must regenerate drums when tables vanish
# (same gate as host/build_host.sh). Otherwise stereo / drum-pan fail to compile.
set -eu
cd "$(dirname "$0")/../.."
root=$(pwd -P)
T=$(mktemp -d "${TMPDIR:-/tmp}/x0x-gen-nonfont.XXXXXX")
T=$(cd "$T" && pwd -P)
trap 'rm -rf "$T"' EXIT HUP INT TERM
# Exercise the real runner with a private build tree. Other host compiles may
# read the generated tables and the scheduler markers for the entire suite.
mkdir -p "$T/tree/tests" "$T/tree/build/gen" "$T/nopil" "$T/bank"
cp tests/run_tests.sh "$T/tree/tests/run_tests.sh"
for dir in tools firmware assets host tests/host; do
    ln -s "$root/$dir" "$T/tree/$dir"
done
cd "$T/tree"
: > "$T/bank/bank.txt"
printf '%s\n' 'raise ImportError("Pillow intentionally hidden for host regression")' > "$T/nopil/PIL.py"
printf 'stub\n' > build/gen/x0x_drum_samples.h

# Mask system and user-site Pillow so the nested probe exercises the missing-Pillow path.
# The outer qualification preflight must not reject this deliberate missing dependency.
out=$(PYTHONPATH="$T/nopil${PYTHONPATH:+:$PYTHONPATH}" PYTHONUSERBASE="$T/nopil" \
    X0X_STRICT_PREFLIGHT=0 X0X_BREAK_BANK="$T/bank" X0X_TEST_JOBS=1 \
    X0X_TESTS='host-build stereo' sh tests/run_tests.sh 2>&1) || {
    printf '%s\n' "$out" | sed 's/^/       /' >&2
    echo "FAIL: host-build/stereo under samples-only + X0X_BREAK_BANK" >&2
    exit 1
}
# Use the same masks as the nested run: PYTHONPATH hides a system-wide Pillow too.
if PYTHONPATH="$T/nopil${PYTHONPATH:+:$PYTHONPATH}" PYTHONUSERBASE="$T/nopil" python3 -c 'import PIL' 2>/dev/null; then
    want='ok   host-build'
else
    want='not-run host-build (Pillow)'
fi
printf '%s\n' "$out" | grep -q "$want" || {
    echo "FAIL: expected $want" >&2
    printf '%s\n' "$out" | sed 's/^/       /' >&2
    exit 1
}
printf '%s\n' "$out" | grep -q 'ok   stereo' || {
    echo "FAIL: expected stereo ok" >&2
    printf '%s\n' "$out" | sed 's/^/       /' >&2
    exit 1
}
[ -f build/gen/x0x_drum_tables.h ] || {
    echo "FAIL: gen_nonfont left x0x_drum_tables.h missing" >&2
    exit 1
}
echo "gen-nonfont-tables: ok"
