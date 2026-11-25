#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")")
b=$(realpath "$1")
(cd "$b"; sha256sum --quiet -c "$p/../controls-init-only/MOD-WHEELS-POST.sha256"; sha256sum --quiet -c "$p/PREIMAGE.sha256")
t=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-s1-rvlfo.XXXXXX")
trap 'rm -rf -- "$t"' EXIT HUP INT TERM
cd "$t"
mkdir out build
for k in firmware tests host; do cp -a --reflink=auto "$b/$k" .; done
cp -a "$b/build/gen" build/
for k in tools assets experiments ble; do ln -s "$b/$k" "$k"; done
patch --batch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$p/s1-init-reverb-lfo.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
sha256sum --quiet -c "$p/../controls-init-only/S1-REVERB-LFO-POST.sha256"
python3 "$p/instrument-host.py" "$t"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_ARP_HOOKS=1 PYTHONDONTWRITEBYTECODE=1
export SMC_UI_TEST_FILE="$p/ui_s1_init_reverb_lfo_test.c" SMC_UI_OUT="$t/out" S1_FILTER_PROJECTS="$t/out"
export SAN_FLAGS="-fsanitize=undefined -fno-sanitize-recover=undefined"
flags="-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -I$t/tests/host"
X0X_HOST_DEFS="$flags" sh tests/host/sj6-smc-ui.sh >new.log 2>&1 || { tail -n 40 new.log; exit 1; }
grep '^PASS' new.log
patch --batch -s -R --fuzz=0 --no-backup-if-mismatch -p1 <"$p/s1-init-reverb-lfo.patch"
for f in firmware/src/app/engine.c firmware/src/dsp/s1c/s1c.c; do cp "$b/$f" "$f"; done
sha256sum --quiet -c "$p/../controls-init-only/MOD-WHEELS-POST.sha256"
X0X_HOST_DEFS="$flags -DS1_REV_OLD=1" sh tests/host/sj6-smc-ui.sh >old.log 2>&1 || { tail -n 40 old.log; exit 1; }
grep '^PASS' old.log
