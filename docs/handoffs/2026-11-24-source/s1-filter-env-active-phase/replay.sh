#!/bin/bash
set -euo pipefail
p=$(realpath "$(dirname "$0")"); a=$(dirname "$p")
b=$(realpath "${1:-$a/controls-init-only/source-mod-wheels}")
d="$a/wheels-autopan-bpm-feedback-classic-typed-join"
w=$(mktemp -d /tmp/dnbox-env-phase.XXXXXX)
trap 'if [[ -s "$w/tree" ]]; then t=$(<"$w/tree"); case "$t" in /tmp/dnbox-s1-mod-sidecar.*) rm -rf -- "$t";;esac;fi;rm -rf -- "$w"' EXIT
(cd "$p";sha256sum -c --quiet DEPENDS.sha256)
awk '/^cp "\$p\/join_classic_all/{exit}{print}' "$d/replay.sh" |
 sed -e "s|^p=\$(cd .*; p0=|p='$d'; p0=|" -e '/^trap /d' -e 's/TMPDIR="$parent" sh/TMPDIR=\/tmp sh/' -e "s|^w=\$(mktemp.*$|w='$w'|" >"$w/stage.sh"
bash "$w/stage.sh" "$b"
cd "$(<"$w/tree")"
sha256sum -c --quiet "$d/POSTIMAGE.sha256"
cp firmware/src/dsp/s1c/s1c.c out/stock-s1c.c
patch -s --fuzz=0 -p1 <"$p/env-active-phase.patch"
sha256sum -c --quiet "$p/POSTIMAGE.sha256"
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >>tests/host/s1_pb_mod_ui.c
cp "$p/s1_env_phase.c" tests/host/
python3 "$p/fixture_panel.py"
python3 "$p/tap.py"
CC='gcc -pipe' sh "$p/run.sh" >out.log 2>&1 || { tail -35 out.log;exit 1; }
grep -E '^(owner=|PASS)' out.log
S1_ENV_UNTOUCHED=out/enabled-default.pcm out/test
cp out/stock-s1c.c firmware/src/dsp/s1c/s1c.c
CC='gcc -pipe' S1_ENV_UNTOUCHED=out/legacy-default.pcm sh "$p/run.sh" >base.log 2>&1 || { tail -30 base.log;exit 1; }
cmp out/enabled-default.pcm out/legacy-default.pcm
printf 'PASS untouched same-joined 4096 stereo frames exact %s\n' "$(sha256sum out/enabled-default.pcm|cut -d' ' -f1)"
