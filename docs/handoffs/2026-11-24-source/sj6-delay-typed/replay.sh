#!/bin/bash
set -euo pipefail
p=$(cd "$(dirname "$0")"&&pwd)
fw=$(dirname "$p")
j="$fw/sj6-mod-pb-typed-join"; a="$fw/all-typed-controls-join"
base=$(realpath "${1:-$fw/controls-init-only/source-mod-wheels}")
w=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-sj6-delay-replay.XXXXXX")
trap 'rm -rf -- "$w"' EXIT
sed -e "s|^p=.*$|p='$a'|" -e "s|\"\$p/isolated.txt\"|'$w/tree'|" "$a/compose.sh" >"$w/compose.sh"
TMPDIR="$w" sh "$w/compose.sh" "$base"
cd "$(<"$w/tree")"
sha256sum --quiet -c "$a/POSTIMAGE.sha256"
patch -s --fuzz=0 -p1 < "$fw/sj6-bend-build9/midi-bend.patch"
patch -s --fuzz=0 -p1 < "$fw/sj6-bend-typed/sj6-pb-typed.patch"
sha256sum --quiet -c "$j/PREIMAGE.sha256"
patch -s --fuzz=0 -p1 < "$j/sj6-mod-pb-typed.patch"
sha256sum --quiet -c "$j/POSTIMAGE.sha256"
sha256sum --quiet -c "$p/PREIMAGE.sha256"
while read -r hash f;do cp --parents "$f" "$w";done <"$p/PREIMAGE.sha256"
patch -s --fuzz=0 -p1 < "$p/delay-typed.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
cp "$p"/sj6_delay_{typed,prior}.c tests/host/
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >>tests/host/s1_pb_mod_ui.c
if ! TMPDIR="$w" PYTHONDONTWRITEBYTECODE=1 sh "$p/run.sh" >"$w/new.log" 2>&1;then tail -45 "$w/new.log";exit 1;fi
grep '^PASS' "$w/new.log"
python3 "$p/measure.py" firmware/src/dsp/juno6_factory.inc
while read -r hash f;do cp "$w/$f" "$f";done <"$p/PREIMAGE.sha256"
sha256sum --quiet -c "$p/PREIMAGE.sha256"
sha256sum --quiet -c "$j/POSTIMAGE.sha256"
rm -f out/test
if ! TMPDIR="$w" PYTHONDONTWRITEBYTECODE=1 sh "$p/run-prior.sh" >"$w/old.log" 2>&1;then tail -45 "$w/old.log";exit 1;fi
grep '^PASS' "$w/old.log"
