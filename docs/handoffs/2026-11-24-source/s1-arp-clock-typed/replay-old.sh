#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")"); a=$(dirname "$p"); b=$(realpath "$1")
c="$a/all-typed-controls-join"; ref="$a/s1-a4-reference-ui"
t=$(mktemp -d "$p/replay.XXXXXX")
trap 'rm -rf -- "$t"' EXIT
sed "s|^p=.*|p='$c'|;s|\$p/isolated.txt|$t/path|" "$c/compose.sh" | TMPDIR="$t" sh -s "$b"
cd "$(cat "$t/path")"
sha256sum --quiet -c "$c/POSTIMAGE.sha256"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$a/s1-a4-reference-receiver/reference.patch"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$ref/a4-reference-ui.patch"
sha256sum --quiet -c "$ref/POSTIMAGE.sha256"
sha256sum --quiet -c "$p/PREIMAGE.sha256"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$p/s1-arp-clock-typed.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
cp firmware/src/app/ui.c out/ui.before-law
cp firmware/src/app/original_presets.inc out/presets.before-law
python3 "$p/original-law.py" "$PWD"
if [ "${LAW_PATCH_OUT:-}" ]; then
    { diff -u --label a/firmware/src/app/ui.c --label b/firmware/src/app/ui.c out/ui.before-law firmware/src/app/ui.c || test "$?" = 1
      diff -u --label a/firmware/src/app/original_presets.inc --label b/firmware/src/app/original_presets.inc out/presets.before-law firmware/src/app/original_presets.inc || test "$?" = 1
    } >"$LAW_PATCH_OUT"
fi
cp "$p/typed-clock.c" tests/host/
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >>tests/host/s1_pb_mod_ui.c
sh "$p/run-new.sh" >new.log 2>&1 || { tail -40 new.log; exit 1; }
grep '^PASS' new.log
CLOCK_KEYBOARD_ONLY=1 SMC_UI_OUT="$PWD/out/keybed" SMC_UI_TEST_FILE=tests/host/ui_arp_keyboard_test.c sh "$p/run-new.sh" >keybed.log 2>&1 || { tail -40 keybed.log; exit 1; }
grep "^PASS" keybed.log

# No product code is instrumented: restore original product tree files in
# place, then compile the original storage and extracted ED_PROJECT handler.
j="$a/sj6-id137-joined-carriage"
while read -r hash file; do cp "$b/$file" "$file"; done <"$j/PREIMAGE.sha256"
while read -r hash file; do test ! -e "$b/$file" || cp "$b/$file" "$file"; done <"$p/PREIMAGE.sha256"
for f in host/x0x_host.c firmware/src/app/original_sound.c firmware/src/app/s1_patch.h firmware/src/app/s1c_part.c firmware/src/app/s1c_part.h firmware/src/dsp/s1c/s1c.c firmware/src/dsp/s1c/s1c_front.c firmware/src/dsp/s1c/s1c_front.h; do cp "$b/$f" "$f"; done
rm firmware/src/app/sj6_137_host_witness.h
sha256sum --quiet -c "$j/PREIMAGE.sha256"
(cd "$b"; sha256sum --quiet -c ../MOD-WHEELS-POST.sha256)
python3 "$j/extract-old-ed-project.py" firmware/src/app/editor.c out/old-ed-project-generated.inc
sh "$j/run-old.sh" >old.log 2>&1 || { tail -40 old.log; exit 1; }
grep '^PASS' old.log
