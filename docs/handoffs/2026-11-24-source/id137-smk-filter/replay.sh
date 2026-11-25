#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")")
j=$(realpath "$p/../sj6-id137-joined-carriage")
t=$(realpath "$p/../id137-generation-bind")
b=$(realpath "$1")
w=$(mktemp -d "$p/.replay.XXXXXX")
trap 'rm -rf -- "$w"' EXIT
(cd "$b"; sha256sum --quiet -c ../MOD-WHEELS-POST.sha256; sha256sum --quiet -c "$j/PREIMAGE.sha256"; sha256sum --quiet -c "$j/GENERATED.sha256")
cd "$w"
for dir in firmware tests host build experiments; do cp -a "$b/$dir" .; done
for dir in tools assets ble; do ln -s "$b/$dir"; done
cp "$j"/sj6_137_*.c "$j"/ui_vnext_old_usb_witness.inc tests/host
patch --fuzz=0 -p1 -s <"$j/combined-host.patch"
sha256sum --quiet -c "$j/POSTIMAGE.sha256"
patch --fuzz=0 -p1 -s <"$t/tuple-binding.patch"
patch --fuzz=0 -p1 -s <"$t/torn-host-tests.patch"
patch --fuzz=0 -p1 -s <"$p/filter-mod-ui.patch"
patch --fuzz=0 -p1 -s <"$p/filter-mod-host.patch"
sh "$j/run-new.sh" >fixed.log 2>&1 || { tail -25 fixed.log; exit 1; }
grep '^PASS' fixed.log
while read -r hash file; do cp "$b/$file" "$file"; done <"$j/PREIMAGE.sha256"
cp "$b/host/x0x_host.c" host/x0x_host.c
rm firmware/src/app/sj6_137_host_witness.h
sha256sum --quiet -c "$j/PREIMAGE.sha256"
python3 "$j/extract-old-ed-project.py" firmware/src/app/editor.c out/old-ed-project-generated.inc
sh "$j/run-old.sh" >old.log 2>&1 || { tail -25 old.log; exit 1; }
grep '^PASS' old.log
