#!/bin/sh
set -eu
q=$(realpath "$(dirname "$0")");b=$(realpath "$1")
t=$(mktemp -d "${TMPDIR:-/tmp}/dn-pages24.XXXXXX")
trap 'rm -rf -- "$t"' EXIT HUP INT TERM
mkdir -p "$t/build" "$t/out"
for k in firmware tests host; do cp -a --reflink=auto "$b/$k" "$t/"; done
cp -a "$b/build/gen" "$t/build/"
for k in tools assets experiments ble; do ln -s "$b/$k" "$t/$k"; done
cd "$t"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_ARP_HOOKS=1 PYTHONDONTWRITEBYTECODE=1
export SMC_UI_TEST_FILE="$q/page-census.c" SMC_UI_OUT="$t/out"
export SAN_FLAGS="-fsanitize=undefined -fno-sanitize-recover=all"
flags="-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DCOMPACT_PAGE_BUDGET=1 -I$t/tests/host"
X0X_HOST_DEFS="$flags" sh tests/host/sj6-smc-ui.sh >before.log 2>before.err || { grep -E 'error:|FAIL|runtime error' before.err | head; exit 1; }
patch --batch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$q/more-pages.patch"
X0X_HOST_DEFS="$flags -DDNBX_PAGE24=1" sh tests/host/sj6-smc-ui.sh >after.log 2>after.err || { grep -E 'error:|FAIL|runtime error' after.err | head; exit 1; }
for arm in before after; do
 grep -E '^(frame|page |list |global )' "$arm.log" >"$arm.census"
 grep -E '^(page-bytes=|checks=)' "$arm.err"
done
cmp before.census after.census || { diff -u before.census after.census | head -n 36; exit 1; }; wc -l after.census; sha256sum after.census
grep '^PASS' after.log
