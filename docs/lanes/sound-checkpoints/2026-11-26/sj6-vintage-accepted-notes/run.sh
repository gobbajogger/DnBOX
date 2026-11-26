#!/usr/bin/env bash
set -euo pipefail
D=$PWD/sj6-native-vintage-owner B=$PWD/sj6-cross-preset T=$PWD/sj6-hp-exact-exp-fit/kept-elite-1
O=$(mktemp -d); trap 'rm -rf "$O"' EXIT
C=/tmp/sj6-cross-range.c
echo "fd43641d63a755ece2b7446fcbf463b2a8b8dd7f3d28df72ef9dcece46c29e9e  $C" |sha256sum -c >&2
python3 sj6-mixer-diagnostic/variant.py "$C" "$O/core.c" center
python3 sj6-solidify-sine-phase/phase.py "$O/core.c"
python3 "$D/bridge.py" "$O/core.c" "$T/firmware/src/dsp" "$O"
python3 "$B/meter.py" "$T/firmware/src/app/original_runtime.c" "$O/runtime.c"
python3 "$D/seed_driver.py" "$B/render.c" "$O/render.c"
for n in 0 1; do
 SJ6_EXTRA_FLAGS="-DDNBX_HOST_VINTAGE_LEGATO=$n -DDNBX_HOST_SINE_PHASE=2 -I$PWD/sj6-wave-carrier" \
 SJ6_RENDER_SOURCE="$O/render.c" SJ6_DNBX_CORE_OVERRIDE="$O/core.c" SJ6_RUNTIME_OVERRIDE="$O/runtime.c" SJ6_JUNO_OVERRIDE="$O/juno.c" \
 bash "$PWD/sj6-joined-dry/build.sh" "$T" "$O/$n" >&2
 for k in 0-dry 1-dry 1-authored 2-dry; do
    "$O/$n" "$B/sub-range.private-output/$k.private.txt" "$O/$n-$k.f32" 2>"$O/$n-$k.log"
 done
done
OPENBLAS_NUM_THREADS=1 python3 "$D/score.py" "$O" "$B"
OPENBLAS_NUM_THREADS=1 python3 "$D/seeds.py" "$O" "$B" >"$D/seeds.json"
