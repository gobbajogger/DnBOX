#!/usr/bin/env bash
set -euo pipefail
root=$(realpath "$1"); packet=$(cd "$(dirname "$0")"&&pwd); parent=$(dirname "$packet")
stage=$(mktemp -d "$(dirname "$root")/sj6-selector-replay.XXXXXX")
w=$(mktemp -d "${TMPDIR:-/tmp}/sj6-selector-fixture.XXXXXX")
trap 'rm -rf -- "$stage" "$w"' EXIT
cp -a --reflink=auto "$root/." "$stage/"
(cd "$stage";sha256sum -c "$packet/APP-PRE.sha256")
for f in firmware/src/{app/{ui.c,engine.c,engine.h,original_runtime.c},dsp/{juno6.h,juno6_factory.inc}}; do rm "$stage/$f"; cp "$root/$f" "$stage/$f"; done
(cd "$stage";for lane in vibrato lfo2 cutoff lfo1-pwm lfo1-cutoff;do patch -s --fuzz=0 -p1 < "$(find "$parent/sj6-mod-wheel-$lane" -maxdepth 1 -name '*.patch')";done
patch -s --fuzz=0 -p1 < "$packet/alt3-recompose.patch")
cp "$packet/test.c" "$w/"
sed -n '/^static int smk_pitch_parameter(int v)$/,/^}$/p; /^static void smk_wheels(void)$/,/^}$/p' "$stage/firmware/src/app/main_fm1.c" > "$w/wheels.inc"
if bash "$packet/run.sh" "$stage" "$w" > "$w/red" 2>&1; then echo 'unexpected pass without selector'; exit 1; fi
grep -F 'pgidx>=0' "$w/red"
(cd "$stage"; patch -s --fuzz=0 -p1 < "$packet/selector-host.patch"; sha256sum -c "$packet/POSTIMAGE.sha256")
if ! bash "$packet/run.sh" "$stage" "$w" > "$w/green" 2>&1;then tail -25 "$w/green";exit 1;fi
grep -F 'PASS physical original MOD selector' "$w/green"
