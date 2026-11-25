#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")")
a=$(dirname "$p")
j="$a/sj6-id137-joined-carriage"
m="$a/s1-mod-rate-probe"
b=$(realpath "$1")
test "$(sha256sum "$b/../MOD-WHEELS-POST.sha256" | cut -d' ' -f1)" = b71f366f8da6ed85a64dbd3ba90b4c2784503511a01558c81ead2ee53e037e3d
(cd "$b"; sha256sum --quiet -c ../MOD-WHEELS-POST.sha256; sha256sum --quiet -c "$j/PREIMAGE.sha256")
t=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-s1-mod-sidecar.XXXXXX")
trap 'rm -rf "$t"' EXIT HUP INT TERM
cd "$t"
mkdir build out
for dir in firmware tests host; do cp -a --reflink=auto "$b/$dir" .; done
cp -a "$b/build/gen" build/
for dir in tools assets experiments ble; do ln -s "$b/$dir"; done
cp "$p/s1_mod_sidecar.c" "$j/sj6_137_old.c" "$j/ui_vnext_old_usb_witness.inc" tests/host/
patch -s --fuzz=0 -p1 <"$j/combined-host.patch"
sha256sum --quiet -c "$j/POSTIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$m/cc1-rate-probe.patch"
patch -s --fuzz=0 -p1 <"$p/s1-mod-sidecar.patch"
awk '/^static int smk_pitch_parameter\(/ {p=1} p {print} p && /^}/ {exit}' firmware/src/app/main_fm1.c >out/mod-wheels.inc
awk '/^static void smk_wheels\(void\)$/ {p=1} p {print} p && /^}/ {exit}' firmware/src/app/main_fm1.c >>out/mod-wheels.inc
sh "$p/run-new.sh" >new.log 2>&1 || { tail -40 new.log; exit 1; }
grep '^PASS' new.log
while read -r hash file; do cp "$b/$file" "$file"; done <"$j/PREIMAGE.sha256"
for f in firmware/src/app/s1c_part.c firmware/src/app/s1c_part.h firmware/src/dsp/s1c/s1c_front.c firmware/src/dsp/s1c/s1c_front.h; do cp "$b/$f" "$f"; done
rm firmware/src/app/sj6_137_host_witness.h
sha256sum --quiet -c "$j/PREIMAGE.sha256"
python3 "$j/extract-old-ed-project.py" firmware/src/app/editor.c out/old-ed-project-generated.inc
sh "$j/run-old.sh" >old.log 2>&1 || { tail -40 old.log; exit 1; }
grep '^PASS' old.log
