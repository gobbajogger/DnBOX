#!/bin/sh
set -eu
d=$(realpath "$(dirname "$0")")
a=$(dirname "$d")
b=$(realpath "${1:-$a/controls-init-only/source-mod-wheels}")
t=$(mktemp -d "$d/replay.XXXXXX")
trap 'rm -rf -- "$t"' EXIT
bash "$d/prep.sh" "$b" "$t" >/dev/null
cd "$(cat "$t/tree")"
sha256sum --quiet -c "$d/PREIMAGE.sha256"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$d/classic-i-smk.patch"
sha256sum --quiet -c "$d/POSTIMAGE.sha256"
mkdir -p firmware/src/dsp/classic_i
for f in classic.c classic.h wrapper.c wrapper.h; do cp "$a/sj6-legacy-chorus-core/$f" firmware/src/dsp/classic_i/; done
(cd firmware/src/dsp/classic_i && sha256sum --quiet -c "$d/CORE.sha256")
cp "$d/classic-smk.c" tests/host/
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >>tests/host/s1_pb_mod_ui.c
sh "$d/run-smk.sh" >"$t/new.log" 2>&1 || { tail -30 "$t/new.log"; exit 1; }
grep '^PASS' "$t/new.log"
patch -R -s --fuzz=0 --no-backup-if-mismatch -p1 <"$d/classic-i-smk.patch"
sha256sum --quiet -c "$d/PREIMAGE.sha256"
rm -rf firmware/src/dsp/classic_i
cp "$d/classic-prior.c" tests/host/
sh "$d/run-prior.sh" >"$t/prior.log" 2>&1 || { tail -30 "$t/prior.log"; exit 1; }
grep '^PASS' "$t/prior.log"
