#!/usr/bin/env bash
set -euo pipefail
D=$PWD/sj6-solidify-tail-slope B=$PWD/sj6-cross-preset T=$PWD/sj6-hp-exact-exp-fit/kept-elite-1
O=$(mktemp -d); trap 'rm -rf "$O"' EXIT
C=/tmp/sj6-cross-range.c
echo "fd43641d63a755ece2b7446fcbf463b2a8b8dd7f3d28df72ef9dcece46c29e9e  $C" |sha256sum -c >&2
python3 "$B/meter.py" "$T/firmware/src/app/original_runtime.c" "$O/runtime.c"
python3 "$D/tap.py" "$B/render.c" "$O/render.c"
for center in 1;do
 cp "$C" "$O/core.c"
 if [ "$center" = 1 ];then
    python3 sj6-mixer-diagnostic/variant.py "$C" "$O/core.c" center
 fi
 python3 "$D/phase.py" "$O/core.c"
 for profile in 0 2; do
  SJ6_DNBX_CORE_OVERRIDE="$O/core.c" SJ6_RUNTIME_OVERRIDE="$O/runtime.c" SJ6_RENDER_SOURCE="$O/render.c" \
  SJ6_EXTRA_FLAGS="-DDNBX_HOST_SINE_PHASE=$profile -I$PWD/sj6-wave-carrier" bash "$B/build.sh" "$T" "$O/bench" >&2
  for f in "$B"/sub-range.private-output/1-*.private.txt;do
    key=$(basename "$f" .private.txt); SJ6_STAGE="$O/$center-$profile-$key" "$O/bench" "$f" "$O/$center-$profile-$key.f32" 2>"$O/$center-$profile-$key.txt"
  done
 done
done
OPENBLAS_NUM_THREADS=1 python3 "$D/score.py" "$O" "$B"
