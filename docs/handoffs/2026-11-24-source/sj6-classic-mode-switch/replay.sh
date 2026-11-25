#!/bin/bash
set -euo pipefail
w=$(cd "$(dirname "$0")"/../..;pwd)
d=$w/perf-final-proof/sj6-classic-mode-switch
p=$w/perf-final-proof/wheels-autopan-bpm-feedback-classic-typed-join
ref=$w/perf-final-proof/sj6-classic-modes-oracle
s=$(realpath "${1:?exact composed Classic I source tree}")
(cd "$s"; sha256sum --quiet -c "$p/POSTIMAGE.sha256")
t=$(mktemp -d /tmp/sj6-mode-switch.XXXXXX)
trap 'rm -rf -- "$t"' EXIT
cp -as "$s/firmware" "$s/tests" "$s/ble" "$s/tools" "$s/assets" "$s/experiments" "$s/host" "$t"
mkdir -p "$t/build" "$t/out"
ln -s "$s/build/gen" "$t/build/gen"
cd "$t"
cp "$s/out/mod-wheels.inc" out/
for f in wrapper.c wrapper.h; do
 rm firmware/src/dsp/classic_i/"$f"
 cp "$ref/$f" firmware/src/dsp/classic_i/"$f"
done
for f in mode-switch.c classic-smk.c; do rm -f tests/host/"$f"; done
cp "$d/mode-switch.c" "$w/perf-final-proof/sj6-classic-i-smk/classic-smk.c" tests/host/
sed -i 's/^int main(void)/int classic_i_primary_main(void)/' tests/host/classic-smk.c
sed 's/-DX0X_SJ6_CLASSIC_I_PROBE=1/-DX0X_SJ6_CLASSIC_I_PROBE=1 -DX0X_SJ6_CLASSIC_SWITCH_WITNESS=1/; s@/classic-smk.c@/mode-switch.c@' "$p/run-classic-join.sh" > run.sh
python3 "$d/bridge.py"
CC='cc -pipe' sh run.sh
