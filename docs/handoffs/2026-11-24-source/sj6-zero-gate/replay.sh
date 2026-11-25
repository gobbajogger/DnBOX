#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")")
a=$(dirname "$p")
all="$a/all-typed-controls-join"
j="$a/sj6-id137-joined-carriage"
b=$(realpath "${1:-$a/controls-init-only/source-mod-wheels}")
(cd "$p"; sha256sum --quiet -c DEPENDS.sha256)
lock=$(mktemp "$p/host-tree.XXXXXX")
script=$(mktemp "$p/assemble.XXXXXX")
trap 'if [ -s "$lock" ]; then t=$(cat "$lock"); case "$t" in */dnbox-s1-mod-sidecar.*) rm -rf -- "$t";; esac; fi; rm -f -- "$lock" "$script"' EXIT
# Reuse exact verified historical composer without touching its own tracking file.
sed -e "s|^p=.*$|p='$all'|" -e "s|\"\$p/isolated.txt\"|'$lock'|" "$all/compose.sh" > "$script"
sh "$script" "$b"
t=$(cat "$lock")
cd "$t"
sha256sum --quiet -c "$all/POSTIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$p/gate-zero.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
cp "$p/ui_gate_zero.c" tests/host/
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >> tests/host/s1_pb_mod_ui.c
sh "$p/run-new.sh" > new.log 2>&1 || { tail -35 new.log; exit 1; }
grep '^PASS' new.log
while read -r hash f; do cp "$b/$f" "$f"; done <"$j/PREIMAGE.sha256"
for f in host/x0x_host.c firmware/src/app/original_sound.c firmware/src/app/s1_patch.h firmware/src/app/s1c_part.c firmware/src/app/s1c_part.h firmware/src/app/engine.c firmware/src/app/engine_arp.inc firmware/src/app/original_runtime.c firmware/src/dsp/s1c/s1c_front.c firmware/src/dsp/s1c/s1c_front.h; do cp "$b/$f" "$f"; done
rm firmware/src/app/sj6_137_host_witness.h
sha256sum --quiet -c "$b/../MOD-WHEELS-POST.sha256"
sha256sum --quiet -c "$j/PREIMAGE.sha256"
python3 "$j/extract-old-ed-project.py" firmware/src/app/editor.c out/old-ed-project-generated.inc
sh "$j/run-old.sh" > old.log 2>&1 || { tail -35 old.log; exit 1; }
grep '^PASS' old.log
