#!/bin/bash
set -euo pipefail
p=$(realpath "$(dirname "$0")"); a=$(dirname "$p")
d="$a/wheels-page24-classic-random2-join"; c="$a/typed-controls-native-admission"
b=$(realpath "${1:-$a/controls-init-only/source-mod-wheels}")
w=$(mktemp -d "$p/stage.XXXXXX")
trap 'rm -rf -- "$w"' EXIT
awk '/^export TMPDIR=/{exit}{print}' "$d/replay.sh" |
  sed -e "s|^p=\$(cd .*; p0=|p='$d'; p0=|" -e '/^trap /d' -e "s|^w=\$(mktemp.*$|w='$w'|" -e "s|TMPDIR=\"\$parent\" sh|TMPDIR='$w' sh|" >"$w/materialize.sh"
bash "$w/materialize.sh" "$b"
cd "$(<"$w/tree")"
apply(){ patch --no-backup-if-mismatch --fuzz=0 -p1 -s "$@"; }
sha256sum -c --quiet "$c/PREIMAGE.sha256"
apply <"$c/native-trial-carrier.patch"
sha256sum -c --quiet "$c/POSTIMAGE.sha256"
sha256sum -c --quiet "$p/PREIMAGE.sha256"
apply <"$p/id137-owner-b39.patch"
sha256sum -c --quiet "$p/POSTIMAGE.sha256"
flags='-Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen -Iexperiments/original_synth -Iexperiments/wobble -DX0X_ORIGINAL_SYNTH=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1'
on='-DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_DELAY_TRACK_TEST=1'
for f in project engine; do cc $flags $on -std=gnu11 -fsyntax-only firmware/src/app/"$f.c"; done
echo "PASS actual native-profile C syntax without X0X_HOST (no target link)"
cp "$p/fixture.c" tests/host/dict_fixture.c
export CC='gcc -pipe' TMPDIR="$w"
for test in now trial prior; do
 if [[ $test == prior ]]; then
   apply -R <"$p/id137-owner-b39.patch"
   sha256sum -c --quiet "$p/PREIMAGE.sha256"
 fi
 sh "$p/run.sh" "$test" >"$w/$test.log" 2>&1 || { tail -30 "$w/$test.log";exit 1; }
 grep '^PASS' "$w/$test.log"
done
