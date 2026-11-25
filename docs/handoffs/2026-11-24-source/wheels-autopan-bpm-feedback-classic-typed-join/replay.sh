#!/bin/bash
set -euo pipefail
p=$(cd "$(dirname "$0")"&&pwd); p0="$p/../wheels-activegate-crush-typed-join"; p1="$p/../wheels-activegate-crush-delay-typed-join"; p2="$p/../wheels-activegate-crush-delay-autopan-typed-join"; p3="$p/../wheels-autopan-bpm-feedback-typed-join"; p4="$p"; p="$p/../wheels-clock-gate-trans-typed-join"; p=$(realpath "$p"); parent=$(dirname "$p"); a="$parent/all-typed-controls-join"; old="$parent/sj6-bend-typed"; wheels="$parent/sj6-mod-pb-typed-join"
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
cp "$p/join_classic_all.c" "$p/classic_mix_prior.c" "$parent/sj6-classic-i-smk/classic-smk.c" tests/host/
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >>tests/host/s1_pb_mod_ui.c
export TMPDIR="$PWD/out/tmp" PYTHONDONTWRITEBYTECODE=1 CC='cc -pipe'
sh "$p/run-classic-join.sh" >"$w/classic.log" 2>&1 || { tail -40 "$w/classic.log"; exit 1; }
grep '^PASS' "$w/classic.log"
cp firmware/src/app/engine.c out/classic-engine.c
cp firmware/src/dsp/s1c/s1c.c out/classic-s1c.c
python3 "$parent/s1-init-reverb-lfo/instrument-host.py" "$PWD"
sh "$p/run-classic-mix.sh" >"$w/mix.log" 2>&1 || { tail -40 "$w/mix.log"; exit 1; }
grep -E '^(PASS|gate=|pages owner=|pan owner=)' "$w/mix.log"
cp out/classic-engine.c firmware/src/app/engine.c
cp out/classic-s1c.c firmware/src/dsp/s1c/s1c.c
(cd out/prior-classic-source; tar cf - $list)|tar xf -
sha256sum --quiet -c "$p/PREIMAGE.sha256"
sh "$p/run-classic-prior.sh" >"$w/prior.log" 2>&1 || { tail -40 "$w/prior.log"; exit 1; }
grep '^PASS' "$w/prior.log"
