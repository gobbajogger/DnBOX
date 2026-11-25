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
patch -s --fuzz=0 -p1 <"$a/s1-transpose-148/transpose-ui.patch"
sha256sum -c --quiet "$a/s1-transpose-148/POSTIMAGE.sha256"
cp firmware/src/dsp/s1c/s1c.c out/legacy-s1c.c
patch -s --fuzz=0 -p1 <"$p/crush-cadence.patch"
patch -s --fuzz=0 -p1 <"$p/crush-ui.patch"
sha256sum -c --quiet "$p/POSTIMAGE.sha256"
cp "$p/s1_crush_lfo.c" "$p/s1_crush_ui.c" tests/host/
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >>tests/host/s1_pb_mod_ui.c
dsp=firmware/src/dsp/s1c/s1c.c
cp "$dsp" out/production-s1c.c
python3 "$p/tap.py"
sh "$p/run-dsp.sh" >dsp.log 2>&1 || { tail -30 dsp.log; exit 1; }
grep -E '^(PASS|route=)' dsp.log
S1_CRUSH_UNTOUCHED=out/active-default.pcm out/test
cp out/production-s1c.c "$dsp"
sh "$p/run-ui.sh" >ui.log 2>&1 || { tail -30 ui.log; exit 1; }
grep '^PASS' ui.log
cp out/legacy-s1c.c "$dsp"
python3 "$p/tap.py"
S1_CRUSH_UNTOUCHED=out/legacy-default.pcm sh "$p/run-dsp.sh" >base.log 2>&1 || { tail -30 base.log; exit 1; }
cmp out/active-default.pcm out/legacy-default.pcm
printf 'PASS untouched same-source 4096-frame stereo PCM exact: %s\n' "$(sha256sum out/active-default.pcm | cut -d' ' -f1)"
while read -r hash file; do cp "$b/$file" "$file"; done <"$j/PREIMAGE.sha256"
for f in host/x0x_host.c firmware/src/app/original_sound.c firmware/src/app/s1_patch.h firmware/src/app/s1c_part.c firmware/src/app/s1c_part.h firmware/src/dsp/s1c/s1c.c firmware/src/dsp/s1c/s1c_front.c firmware/src/dsp/s1c/s1c_front.h; do cp "$b/$f" "$f"; done
rm firmware/src/app/sj6_137_host_witness.h
sha256sum -c --quiet "$j/PREIMAGE.sha256"
python3 "$j/extract-old-ed-project.py" firmware/src/app/editor.c out/old-ed-project-generated.inc
sh "$j/run-old.sh" >old.log 2>&1 || { tail -30 old.log; exit 1; }
grep '^PASS' old.log
