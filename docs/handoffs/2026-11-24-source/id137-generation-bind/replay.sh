#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")")
j=$(realpath "$p/../sj6-id137-joined-carriage")
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
sh "$j/run-new.sh" >base.log 2>&1 || { tail -25 base.log; exit 1; }
grep -E '^(PASS|EXPECTED RED)' base.log
patch --fuzz=0 -p1 -s <"$p/tuple-binding.patch"
patch --fuzz=0 -p1 -s <"$p/torn-host-tests.patch"
sh "$j/run-new.sh" >fixed.log 2>&1 || { tail -25 fixed.log; exit 1; }
grep '^PASS' fixed.log
