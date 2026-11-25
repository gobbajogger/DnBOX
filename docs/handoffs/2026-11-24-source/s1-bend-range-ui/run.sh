#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")")
b=$(realpath "$1")
t=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-s1-bend.XXXXXX")
trap 'rm -rf -- "$t"' EXIT HUP INT TERM
(cd "$b"; sha256sum --quiet -c "$p/../controls-init-only/MOD-WHEELS-POST.sha256")
cd "$t"
mkdir out build
for k in firmware tests host; do cp -a --reflink=auto "$b/$k" .; done
cp -a "$b/build/gen" build/
for k in tools assets experiments ble; do ln -s "$b/$k" "$k"; done
patch --batch --fuzz=0 --no-backup-if-mismatch -p1 <"$p/wheels-bend-ui.patch"
awk '/^static int smk_pitch_parameter\(/ {p=1} p {print} p && /^}/ {exit}' firmware/src/app/main_fm1.c >out/mod-wheels.inc
awk '/^static void smk_wheels\(void\)$/ {p=1} p {print} p && /^}/ {exit}' firmware/src/app/main_fm1.c >>out/mod-wheels.inc
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_ARP_HOOKS=1 PYTHONDONTWRITEBYTECODE=1
export SMC_UI_TEST_FILE="$p/wheel-range-ui-host.c" SMC_UI_OUT="$t/out"
export SAN_FLAGS="-fsanitize=shift -fno-sanitize-recover=shift"
export X0X_HOST_DEFS="-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DX0X_S1_BEND_RANGE_PROBE=1 -I$t/tests/host -I$t/out -Wl,--wrap=s1_part_init"
sh tests/host/sj6-smc-ui.sh >run.log 2>&1 || { tail -n 40 run.log; exit 1; }
grep '^PASS' run.log
