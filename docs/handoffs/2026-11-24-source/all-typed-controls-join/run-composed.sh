#!/bin/sh
set -eu
p=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
a=$(dirname "$p"); j="$a/sj6-id137-joined-carriage"; bend="$a/s1-bend-typed"
b=$(realpath "$1")
cd "$2"
cp "$p/mixed_typed_controls.c" tests/host/mixed_typed_controls.c
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo "#endif" >> tests/host/s1_pb_mod_ui.c
run() {
  name=$1; test=$2; wrap=$3
  sed "s@-Wl,--wrap=s1_part_init@$wrap@;s@s1_pb_mod_ui.c@$test@" "$bend/run-new.sh" >out/run-test.sh
  sh out/run-test.sh >"$name.log" 2>&1 || { tail -30 "$name.log"; exit 1; }
  grep '^PASS' "$name.log"
}
run pb s1_pb_mod_ui.c '-Wl,--wrap=s1_part_init'
run sj6 sj6_137_carriage.c '-Wl,--wrap=juno6_init'
cp firmware/src/dsp/s1c/s1c.c out/prod-s1c.c
cp firmware/src/app/engine.c out/prod-engine.c
python3 "$a/s1-init-reverb-lfo/instrument-host.py" "$PWD"
run reverb ui_s1_init_reverb_lfo_test.c ''
mv out/prod-s1c.c firmware/src/dsp/s1c/s1c.c
mv out/prod-engine.c firmware/src/app/engine.c
run mixed mixed_typed_controls.c '-Wl,--wrap=s1_part_init -Wl,--wrap=juno6_init'
cp out/mixed-media.bin out/media.bin
while read -r hash file; do cp "$b/$file" "$file"; done <"$j/PREIMAGE.sha256"
for f in host/x0x_host.c firmware/src/app/original_sound.c firmware/src/app/s1_patch.h firmware/src/app/s1c_part.c firmware/src/app/s1c_part.h firmware/src/dsp/s1c/s1c_front.c firmware/src/dsp/s1c/s1c_front.h; do cp "$b/$f" "$f"; done
rm firmware/src/app/sj6_137_host_witness.h
sha256sum --quiet -c "$j/PREIMAGE.sha256"
python3 "$j/extract-old-ed-project.py" firmware/src/app/editor.c out/old-ed-project-generated.inc
sh "$j/run-old.sh" > old.log 2>&1 || { tail -35 old.log; exit 1; }
grep '^PASS' old.log
