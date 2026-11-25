#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")");b=$(realpath "$1")
t=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-a4.XXXXXX")
trap 'rm -rf -- "$t"' EXIT HUP INT TERM
cd "$t"
(cd "$b"; sha256sum --quiet -c "$p/PREIMAGE.sha256")
mkdir out build
for k in firmware tests host; do cp -a --reflink=auto "$b/$k" .; done
cp -a "$b/build/gen" build/
for k in tools assets experiments ble; do ln -s "$b/$k" "$k"; done
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_ARP_HOOKS=1 PYTHONDONTWRITEBYTECODE=1
export SMC_UI_TEST_FILE="$p/host.c" SMC_UI_OUT="$t/out"
export SAN_FLAGS="-fsanitize=undefined,float-cast-overflow -fno-sanitize-recover=all"
export X0X_HOST_DEFS="-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -I$t/tests/host -Wl,--wrap=s1_part_init"
build() { "$@" sh tests/host/sj6-smc-ui.sh >test.log 2>&1 || { tail -40 test.log; exit 1; }; }
build env DNBOX_A4_UNEDITED="$t/before.pcm"
if "$t/out/test" >red.log 2>&1; then echo 'expected unmapped original reference (red)';exit 1;fi
grep '^FAIL ' red.log
patch --batch --fuzz=0 --no-backup-if-mismatch -p1 <"$p/reference.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
build env DNBOX_A4_UNEDITED="$t/after.pcm"
cmp before.pcm after.pcm; sha256sum after.pcm
"$t/out/test"
