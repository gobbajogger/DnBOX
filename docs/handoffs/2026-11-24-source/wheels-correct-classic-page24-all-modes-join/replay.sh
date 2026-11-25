#!/bin/bash
set -euo pipefail
p=$(cd "$(dirname "$0")"&&pwd); p0="$p/../wheels-activegate-crush-typed-join"; p1="$p/../wheels-activegate-crush-delay-typed-join"; p2="$p/../wheels-activegate-crush-delay-autopan-typed-join"; p3="$p/../wheels-autopan-bpm-feedback-typed-join"; p4="$p/../wheels-autopan-bpm-feedback-classic-typed-join"; p5="$p/../wheels-correct-classic-typed-join"; p6="$p/../wheels-correct-classic-noise-witness-join"; p7="$p/../wheels-correct-classic-noise-page24-join"; p8="$p"; p="$p/../wheels-clock-gate-trans-typed-join"; p=$(realpath "$p"); parent=$(dirname "$p"); a="$parent/all-typed-controls-join"; old="$parent/sj6-bend-typed"; wheels="$parent/sj6-mod-pb-typed-join"
b=$(realpath "${1:-$parent/controls-init-only/source-mod-wheels}")
w=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-sj6-modpb-replay.XXXXXX")
trap 'if [[ -s "$w/tree" ]];then t=$(<"$w/tree");case "$t" in */dnbox-s1-mod-sidecar.*) rm -rf -- "$t";;esac;fi;rm -rf -- "$w"' EXIT
sed -e "s|^p=.*$|p='$a'|" -e "s|\"\$p/isolated.txt\"|'$w/tree'|" "$a/compose.sh" >"$w/compose.sh"
TMPDIR="$parent" sh "$w/compose.sh" "$b"
t=$(<"$w/tree");cd "$t"
sha256sum --quiet -c "$a/POSTIMAGE.sha256"
patch -s --fuzz=0 -p1 < "$parent/sj6-bend-build9/midi-bend.patch"
patch -s --fuzz=0 -p1 < "$old/sj6-pb-typed.patch"
sha256sum --quiet -c "$old/POSTIMAGE.sha256"
sha256sum --quiet -c "$wheels/PREIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$wheels/sj6-mod-pb-typed.patch"
sha256sum --quiet -c "$wheels/POSTIMAGE.sha256"
a4rv="$parent/wheels-a4-rv-typed-join"
sha256sum --quiet -c "$a4rv/PREIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$a4rv/wheels-a4-rv.patch"
sha256sum --quiet -c "$a4rv/POSTIMAGE.sha256"
sha256sum -c "$p/PREIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$p/full-controls.patch"
sha256sum -c "$p/POSTIMAGE.sha256"
p=$p0
sha256sum --quiet -c "$p/PREIMAGE.sha256"
mkdir -p out/prior-source
list=$(awk '{print $2}' "$p/PREIMAGE.sha256")
tar cf - $list |tar xf - -C out/prior-source
patch -s --fuzz=0 -p1 <"$p/activegate-crush.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
p=$p1
sha256sum --quiet -c "$p/PREIMAGE.sha256"
mkdir -p out/prior-delay-source out/tmp
list=$(awk '{print $2}' "$p/PREIMAGE.sha256")
tar cf - $list |tar xf - -C out/prior-delay-source
patch -s --fuzz=0 -p1 <"$p/delay-sync-join.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
p=$p2
sha256sum --quiet -c "$p/PREIMAGE.sha256"
mkdir -p out/prior-pan-source out/tmp
list=$(awk '{print $2}' "$p/PREIMAGE.sha256")
tar cf - $list |tar xf - -C out/prior-pan-source
patch -s --fuzz=0 -p1 <"$p/autopan-join.patch"
sha256sum --quiet -c "$p/PAN-POSTIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$p/bpm-join.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
p=$p3
sha256sum --quiet -c "$p/PREIMAGE.sha256"
mkdir -p out/prior-feedback-source out/tmp
list=$(awk '{print $2}' "$p/PREIMAGE.sha256")
tar cf - $list |tar xf - -C out/prior-feedback-source
patch -s --fuzz=0 -p1 <"$p/feedback-join.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
p=$p4
sha256sum --quiet -c "$p/PREIMAGE.sha256"
mkdir -p out/prior-classic-source
list=$(awk '{print $2}' "$p/PREIMAGE.sha256")
tar cf - $list |tar xf - -C out/prior-classic-source
patch -s --fuzz=0 -p1 <"$p/classic-join.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
mkdir -p firmware/src/dsp/classic_i
cp "$parent/sj6-legacy-chorus-core/"{classic.c,classic.h,wrapper.c,wrapper.h} firmware/src/dsp/classic_i/
(cd firmware/src/dsp/classic_i;sha256sum --quiet -c "$p/CORE.sha256")
p=$p5
sha256sum --quiet -c "$p/PREIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$p/correct-classic-join.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
(cd firmware/src/dsp/classic_i;sha256sum --quiet -c "$p/CORE.sha256")
p=$p6
sha256sum --quiet -c "$p/PREIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$p/noise-ui-witness.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
p=$p7
sha256sum --quiet -c "$p/PREIMAGE.sha256"
mkdir -p out/page24 out/tmp
mkdir -p out/prior-page24-source out/tmp
pagefiles=$(awk '{print $2}' "$p/PREIMAGE.sha256")
tar cf - $pagefiles |tar xf - -C out/prior-page24-source
patch -s --fuzz=0 -p1 <"$p/page24-join.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
p=$p8
sha256sum --quiet -c "$p/PREIMAGE.sha256"
patch -s --fuzz=0 -p1 <"$p/all-classic-join.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
cp "$p/join_all_modes.c" "$p4/classic_mix_prior.c" "$parent/sj6-classic-all-smk/all-smk.c" tests/host/
sed -e 's/^int main(void)/int classic_i_primary_main(void)/' -e '$i\ return 0;' "$parent/sj6-classic-i-smk/classic-smk.c" >tests/host/classic-smk.c
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >>tests/host/s1_pb_mod_ui.c
export TMPDIR="$PWD/out/tmp" PYTHONDONTWRITEBYTECODE=1 CC='cc -pipe'
sh "$p/run-all-modes.sh" >"$w/modes.log" 2>&1 || { tail -40 "$w/modes.log"; exit 1; }
grep '^PASS' "$w/modes.log"
SJ6_TEST_CLASSIC_I=1 out/test
cp firmware/src/app/engine.c out/all-modes-engine.c
cp firmware/src/dsp/s1c/s1c.c out/all-modes-s1c.c
python3 "$parent/s1-init-reverb-lfo/instrument-host.py" "$PWD"
sh "$p/run-all-modes-mix.sh" >"$w/mixed.log" 2>&1 || { tail -40 "$w/mixed.log"; exit 1; }
grep -E '^(PASS|gate=|pages owner=|pan owner=)' "$w/mixed.log"
cp out/all-modes-engine.c firmware/src/app/engine.c
cp out/all-modes-s1c.c firmware/src/dsp/s1c/s1c.c
(cd out/prior-page24-source;tar cf - $pagefiles)|tar xf -
(cd out/prior-classic-source;tar cf - $(awk '{print $2}' "$p4/PREIMAGE.sha256"))|tar xf -
sha256sum --quiet -c "$p3/POSTIMAGE.sha256"
sha256sum --quiet -c "$p4/PREIMAGE.sha256"
sh "$p4/run-classic-prior.sh" >"$w/prior.log" 2>&1 || { tail -40 "$w/prior.log"; exit 1; }
grep '^PASS' "$w/prior.log"
