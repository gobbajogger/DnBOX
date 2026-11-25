#!/bin/bash
set -euo pipefail
p=$(cd "$(dirname "$0")"&&pwd)
parent=$(dirname "$p")
d="$parent/wheels-page24-classic-random2-join"
b=$(realpath "${1:-$parent/controls-init-only/source-mod-wheels}")
w=$(mktemp -d "$p/stage.XXXXXX")
trap 'rm -rf "$w"' EXIT
awk '/^export TMPDIR=/{exit}{print}' "$d/replay.sh" |
  sed -e "s|^p=\$(cd .*; p0=|p='$d'; p0=|" -e '/^trap /d' -e "s|^w=\$(mktemp.*$|w='$w'|" -e "s|TMPDIR=\"\$parent\" sh|TMPDIR='$w' sh|" >"$w/materialize.sh"
bash "$w/materialize.sh" "$b"
cd "$(<"$w/tree")"
sha256sum -c --quiet "$p/PREIMAGE.sha256"
patch --no-backup-if-mismatch --fuzz=0 -p1 -s <"$p/native-trial-carrier.patch"
sha256sum -c --quiet "$p/POSTIMAGE.sha256"
export CC='gcc -pipe' TMPDIR="$w" TRIAL_INPUT="$w/media"
mkdir -p "$TRIAL_INPUT"
cp "$p/trial_fixture.c" tests/host/
for test in writer trial old; do
  sh "$p/run-trial.sh" "$test" >"$w/$test.log" 2>&1 || { tail -28 "$w/$test.log";exit 1; }
  grep -E '^(PASS|writer owner|trial owner)' "$w/$test.log"
done
cmp "$p/emulator-readonly-autopan.x0xp" "$w/media/admitted.x0xp"
cmp "$p/virtual-store-readonly-autopan.bin" "$w/media/admitted-0.bin"
flags='-Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen -Iexperiments/original_synth -Iexperiments/wobble -DX0X_ORIGINAL_SYNTH=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1'
on='-DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1'
for f in project engine original_runtime s1c_part; do
  cc $flags -std=gnu11 -fsyntax-only firmware/src/app/"$f.c"
  cc $flags $on -std=gnu11 -fsyntax-only firmware/src/app/"$f.c"
done
cc $flags -E -P firmware/src/app/project.c >"$w/off.pp"
cp "$b/firmware/src/app/project.c" firmware/src/app/project-stock-check.tmp
cc $flags -E -P -x c firmware/src/app/project-stock-check.tmp >"$w/stock.pp"
rm firmware/src/app/project-stock-check.tmp
cmp "$w/off.pp" "$w/stock.pp"
if cc $flags -std=gnu11 -fsyntax-only -DDNBOX_137_HOST_WITNESS=1 firmware/src/app/project.c >"$w/denied.log" 2>&1; then exit 2; fi
grep -q 'not enabled in stock' "$w/denied.log"
if cc $flags $on -std=gnu11 -fsyntax-only -DX0X_SJ6_DELAY_WITNESS=1 firmware/src/app/project.c >"$w/denied.log" 2>&1; then exit 3; fi
grep -q 'outside this read-only' "$w/denied.log"
echo 'PASS stock project C preprocessed bytes exact; native C syntax and explicit fail-closed flag contracts'
cp firmware/src/app/engine.c out/admission-engine.c
cp firmware/src/dsp/s1c/s1c.c out/admission-s1c.c
python3 "$parent/s1-init-reverb-lfo/instrument-host.py" "$PWD"
sh "$d/run-random2-mixed.sh" >"$w/full-mixed.log" 2>&1 || { tail -28 "$w/full-mixed.log";exit 1; }
grep '^PASS' "$w/full-mixed.log"
cp out/admission-engine.c firmware/src/app/engine.c
cp out/admission-s1c.c firmware/src/dsp/s1c/s1c.c
patch -R --no-backup-if-mismatch --fuzz=0 -p1 -s <"$p/native-trial-carrier.patch"
sha256sum -c --quiet "$p/PREIMAGE.sha256"
patch -R --no-backup-if-mismatch --fuzz=0 -p1 -s <"$d/random2-join.patch"
sha256sum -c --quiet "$d/PREIMAGE.sha256"
sh "$d/run-random2-prior.sh" >"$w/exact-prior.log" 2>&1 || { tail -28 "$w/exact-prior.log";exit 1; }
grep '^PASS' "$w/exact-prior.log"
