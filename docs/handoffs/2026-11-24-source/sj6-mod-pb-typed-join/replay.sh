#!/bin/bash
set -euo pipefail
p=$(cd "$(dirname "$0")"&&pwd); parent=$(dirname "$p"); a="$parent/all-typed-controls-join"; old="$parent/sj6-bend-typed"
b=$(realpath "${1:-$parent/controls-init-only/source-mod-wheels}")
w=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-sj6-modpb-replay.XXXXXX")
trap 'if [[ -s "$w/tree" ]];then t=$(<"$w/tree");case "$t" in */dnbox-s1-mod-sidecar.*) rm -rf -- "$t";;esac;fi;rm -rf -- "$w"' EXIT
sed -e "s|^p=.*$|p='$a'|" -e "s|\"\$p/isolated.txt\"|'$w/tree'|" "$a/compose.sh" >"$w/compose.sh"
TMPDIR="$parent" sh "$w/compose.sh" "$b"
t=$(<"$w/tree");cd "$t"
sha256sum --quiet -c "$a/POSTIMAGE.sha256"
patch -s --fuzz=0 -p1 < "$parent/sj6-bend-build9/midi-bend.patch"
patch -s --fuzz=0 -p1 < "$old/sj6-pb-typed.patch"
sha256sum --quiet -c "$old/POSTIMAGE.sha256"
sha256sum --quiet -c "$p/PREIMAGE.sha256"
while read -r hash f;do cp --parents "$f" "$w";done <"$p/PREIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$p/sj6-mod-pb-typed.patch"
sha256sum -c "$p/POSTIMAGE.sha256"
cp "$p/sj6_mod_pb_join.c" tests/host
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >> tests/host/s1_pb_mod_ui.c
if ! PYTHONDONTWRITEBYTECODE=1 sh "$p/run.sh" >"$w/new.log" 2>&1; then tail -45 "$w/new.log";exit 1;fi
grep '^PASS' "$w/new.log"
while read -r hash f;do cp "$w/$f" "$f";done <"$p/PREIMAGE.sha256"
sha256sum --quiet -c "$old/POSTIMAGE.sha256"
cp "$p/old.c" tests/host/sj6_mod_pb_prior.c
sed 's,sj6_pb_mixed.c,sj6_mod_pb_prior.c,;s/ -Wl,--wrap=s1_part_init -Wl,--wrap=juno6_init//' "$old/run-new.sh" > "$w/run-old.sh"
rm -f out/test
if ! PYTHONDONTWRITEBYTECODE=1 sh "$w/run-old.sh" >"$w/old.log" 2>&1;then tail -45 "$w/old.log";exit 1;fi
grep '^PASS' "$w/old.log"
