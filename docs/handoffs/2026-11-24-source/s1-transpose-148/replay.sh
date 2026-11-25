#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")"); a=$(dirname "$p"); b=$(realpath "$1")
j="$a/sj6-id137-joined-carriage"; c="$a/all-typed-controls-join"
test ! -e "$c/isolated.txt"
(cd "$p"; sha256sum -c --quiet DEPENDS.sha256)
sh "$c/compose.sh" "$b"
t=$(cat "$c/isolated.txt"); rm "$c/isolated.txt"
trap 'case "$t" in */dnbox-s1-mod-sidecar.*) rm -rf -- "$t";; esac' EXIT
cd "$t"
sha256sum -c --quiet "$c/POSTIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$a/s1-a4-reference-receiver/reference.patch"
patch -s --fuzz=0 -p1 <"$a/s1-a4-reference-ui/a4-reference-ui.patch"
sha256sum -c --quiet "$a/s1-a4-reference-ui/POSTIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$a/s1-id38-rvsize/rvsize-ui.patch"
sha256sum -c --quiet "$a/s1-id38-rvsize/POSTIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$p/transpose-ui.patch"
sha256sum -c --quiet "$p/POSTIMAGE.sha256"
cp "$p/s1_trans_ui.c" tests/host/
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >>tests/host/s1_pb_mod_ui.c
sh "$p/run-new.sh" >new.log 2>&1 || { tail -35 new.log; exit 1; }
grep '^PASS' new.log
while read -r hash file; do cp "$b/$file" "$file"; done <"$j/PREIMAGE.sha256"
for f in host/x0x_host.c firmware/src/app/original_sound.c firmware/src/app/s1_patch.h firmware/src/app/s1c_part.c firmware/src/app/s1c_part.h firmware/src/dsp/s1c/s1c.c firmware/src/dsp/s1c/s1c_front.c firmware/src/dsp/s1c/s1c_front.h; do cp "$b/$f" "$f"; done
rm firmware/src/app/sj6_137_host_witness.h
sha256sum -c --quiet "$j/PREIMAGE.sha256"
python3 "$j/extract-old-ed-project.py" firmware/src/app/editor.c out/old-ed-project-generated.inc
sh "$j/run-old.sh" >old.log 2>&1 || { tail -30 old.log; exit 1; }
grep '^PASS' old.log
