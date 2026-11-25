#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")")
b=$(realpath "$1")
pin=b71f366f8da6ed85a64dbd3ba90b4c2784503511a01558c81ead2ee53e037e3d
actual=$(sha256sum "$b/../MOD-WHEELS-POST.sha256" | cut -d ' ' -f 1)
test "$actual" = "$pin"
(cd "$b"; sha256sum --quiet -c ../MOD-WHEELS-POST.sha256; sha256sum --quiet -c "$p/PREIMAGE.sha256"; sha256sum --quiet -c "$p/GENERATED.sha256")
d=$(mktemp -d /tmp/dnbox-137-joined.XXXXXX)
trap 'rm -rf "$d"' EXIT HUP INT TERM
cd "$d"
for dir in firmware tests host build; do cp -a --reflink=auto "$b/$dir" .; done
for dir in tools assets experiments ble; do ln -s "$b/$dir"; done
cp "$p"/sj6_137_*.c "$p/ui_vnext_old_usb_witness.inc" tests/host/
patch -s --fuzz=0 -p1 <"$p/combined-host.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
sh "$p/run-new.sh" >new.log 2>&1 || { tail -45 new.log; exit 1; }
grep -E '^(PASS|EXPECTED RED)' new.log
while read -r hash file; do cp "$b/$file" "$file"; done <"$p/PREIMAGE.sha256"
rm firmware/src/app/sj6_137_host_witness.h
sha256sum --quiet -c "$p/PREIMAGE.sha256"
python3 "$p/extract-old-ed-project.py" firmware/src/app/editor.c out/old-ed-project-generated.inc
sh "$p/run-old.sh" >old.log 2>&1 || { tail -45 old.log; exit 1; }
grep '^PASS' old.log
