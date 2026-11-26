#!/usr/bin/env bash
set -euo pipefail
D=$PWD/sj6-raw-fixed-ingress B=$PWD/sj6-cross-preset T=$PWD/sj6-hp-exact-exp-fit/kept-elite-1
O=$(mktemp -d); trap 'rm -rf "$O"' EXIT
C=/tmp/sj6-cross-range.c
echo "fd43641d63a755ece2b7446fcbf463b2a8b8dd7f3d28df72ef9dcece46c29e9e  $C" |sha256sum -c >&2
python3 sj6-mixer-diagnostic/variant.py "$C" "$O/center.c" center
python3 "$B/meter.py" "$T/firmware/src/app/original_runtime.c" "$O/runtime.c"
python3 "$D/driver.py" "$B/render.c" "$O/bench.c"
for mode in 0 1;do
 SJ6_EXTRA_FLAGS="-DSJ6_MATCH_FIXED_TAPER=$mode" SJ6_RENDER_SOURCE="$O/bench.c" SJ6_RUNTIME_OVERRIDE="$O/runtime.c" SJ6_DNBX_CORE_OVERRIDE="$O/center.c" \
   bash "$B/build.sh" "$T" "$O/probe" >&2
 for arm in dry authored ;do
   "$O/probe" "$B/sub-range.private-output/2-$arm.private.txt" "$O/$mode-$arm.f32" 2>"$O/$mode-$arm.log"
   SJ6_NO_FIXED_FIXTURE=1 "$O/probe" "$B/sub-range.private-output/2-$arm.private.txt" "$O/$mode-no-$arm.f32" 2>"$O/$mode-no-$arm.log"
 done
done
OPENBLAS_NUM_THREADS=1 python3 "$D/score.py" "$O" "$B"
