#!/bin/sh
set -eu
parent=${1:?perf-final-proof path}
p=${2:?this packet containing exact host runners}
export TMPDIR="$PWD/out/tmp" CC='cc -pipe' PYTHONDONTWRITEBYTECODE=1
mkdir -p out/tmp out/page24 out/page24-guard-off out/page24-guard-on
rm -f out/page24/test
sed 's/^int main(void)/int classic_i_primary_main(void)/' "$parent/sj6-classic-i-smk/classic-smk.c" >tests/host/classic-smk.c
cp "$parent/sj6-classic-mode-switch/mode-switch.c" tests/host/
sh "$p/run-mode-switch-join.sh" >out/page24/modes.log 2>&1 || { tail -40 out/page24/modes.log;exit 1; }
grep '^PASS' out/page24/modes.log
cp firmware/src/app/engine.c out/page24/runtime-engine.c
cp firmware/src/dsp/s1c/s1c.c out/page24/runtime-s1c.c
restore() {
 cp out/page24/runtime-engine.c firmware/src/app/engine.c
 cp out/page24/runtime-s1c.c firmware/src/dsp/s1c/s1c.c
}
trap restore EXIT
python3 "$parent/s1-init-reverb-lfo/instrument-host.py" "$PWD"
sh "$p/run-correct-classic-mix.sh" >out/page24/mixed.log 2>&1 || { tail -40 out/page24/mixed.log;exit 1; }
grep -E '^(PASS|gate=|pages owner=|pan owner=)' out/page24/mixed.log
restore
trap - EXIT
rm -f out/test
NOISE_OUTPUT="$PWD/out/page24-guard-off" sh "$p/run-noise-compose.sh" >out/page24/noise-off.log 2>&1 || { tail -40 out/page24/noise-off.log;exit 1; }
rm -f out/page24-guard-off/test
NOISE_FLAG=-DDNBOX_NOISE_PHYSICAL_WITNESS=1 NOISE_OUTPUT="$PWD/out/page24-guard-on" sh "$p/run-noise-compose.sh" >out/page24/noise-on.log 2>&1 || { tail -40 out/page24/noise-on.log;exit 1; }
for owner in 0 1; do
 cmp "out/page24-guard-off/default-$owner.pcm" "out/page24-guard-on/default-$owner.pcm"
done
for state in off on;do grep -E '^(PASS|owner)' "out/page24/noise-$state.log";done
sha256sum out/page24-guard-on/default-?.pcm
