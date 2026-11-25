#!/bin/bash
set -euo pipefail
p=$(realpath "$(dirname "$0")"); a=$(dirname "$p")
b=$(realpath "${1:-$a/controls-init-only/source-mod-wheels}")
j="$a/sj6-id137-joined-carriage"; d="$a/wheels-activegate-crush-delay-typed-join"
w=$(mktemp -d /tmp/dnbox-df-replay.XXXXXX)
trap 'if [[ -s "$w/t" ]]; then t=$(<"$w/t"); case "$t" in /tmp/dnbox-s1-mod-sidecar.*) rm -rf "$t";;esac;fi; rm -rf "$w"' EXIT
(cd "$p"; sha256sum -c --quiet DEPENDS.sha256)
awk '/^cp "\$p\/join_delay_all/{exit}{print}' "$d/replay.sh" |
 sed -e "s|^p=\$(cd .*; p0=|p='$d'; p0=|" -e '/^trap /d' -e 's/TMPDIR="$parent" sh/TMPDIR=\/tmp sh/' -e "s/^w=\$(mktemp.*$/w='$w'/" >"$w/compose.sh"
echo 'echo "$t" >"$w/t"' >>"$w/compose.sh"
bash "$w/compose.sh" "$b"
rm -f "$w/tree"
cd "$(<"$w/t")"
sha256sum -c --quiet "$d/POSTIMAGE.sha256"
cp firmware/src/dsp/s1c/s1c.c out/stock-s1c.c
patch -s --fuzz=0 -p1 <"$p/feedback-dsp.patch"
patch -s --fuzz=0 -p1 <"$p/feedback-ui.patch"
sha256sum -c --quiet "$p/POSTIMAGE.sha256"
cp "$p/s1_feedback_source.c" "$p/s1_feedback_ui.c" tests/host/
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >>tests/host/s1_pb_mod_ui.c
sh "$p/run-ui.sh" >ui.log 2>&1 || { tail -30 ui.log; exit 1; }
grep '^PASS' ui.log
python3 "$p/tap.py"
sh "$p/run-dsp.sh" >dsp.log 2>&1 || { tail -30 dsp.log; exit 1; }
grep -E '^(PASS|owner[01])' dsp.log
S1_FEEDBACK_UNTOUCHED=out/enabled-default.pcm out/test
cp out/stock-s1c.c firmware/src/dsp/s1c/s1c.c; python3 "$p/tap.py"
S1_FEEDBACK_UNTOUCHED=out/legacy-default.pcm sh "$p/run-dsp.sh" >base.log 2>&1 || { tail -30 base.log; exit 1; }
cmp out/enabled-default.pcm out/legacy-default.pcm
printf 'PASS untouched same-joined 4096 stereo frames exact %s\n' "$(sha256sum out/enabled-default.pcm|cut -d' ' -f1)"
cp -a "$b/firmware/." firmware;cp -a "$b/host/." host
rm firmware/src/app/sj6_137_host_witness.h
sha256sum -c --quiet "$j/PREIMAGE.sha256"
cp "$j/sj6_137_old.c" "$j/ui_vnext_old_usb_witness.inc" tests/host/
python3 "$j/extract-old-ed-project.py" firmware/src/app/editor.c out/old-ed-project-generated.inc
sh "$j/run-old.sh" >old.log 2>&1 || { tail -30 old.log; exit 1; }
grep '^PASS' old.log
