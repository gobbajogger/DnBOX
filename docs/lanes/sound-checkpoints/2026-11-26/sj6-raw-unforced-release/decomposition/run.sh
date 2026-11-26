#!/usr/bin/env bash
set -euo pipefail
D=$PWD/sj6-raw-modal-decomp
B=$PWD/sj6-cross-preset
S=$PWD/sj6-sub-suffix
T=$PWD/sj6-hp-exact-exp-fit/kept-elite-1
C=/tmp/sj6-cross-range.c
O=$(mktemp -d); trap 'rm -rf "$O"' EXIT
echo "fd43641d63a755ece2b7446fcbf463b2a8b8dd7f3d28df72ef9dcece46c29e9e  $C" |sha256sum -c >&2
python3 sj6-mixer-diagnostic/variant.py "$C" "$O/center.c" center
python3 "$S/route.py" "$T/firmware/src/app/original_runtime.c" "$O/runtime.c"
python3 "$PWD/sj6-raw-release-tap/gen.py" "$S" "$O"
SJ6_DNBX_CORE_OVERRIDE="$O/center.c" SJ6_RUNTIME_OVERRIDE="$O/runtime.c" SJ6_RENDER_SOURCE="$O/probe.c" SJ6_EXTRA_FLAGS="-I$S" bash "$B/build.sh" "$T" "$O/probe" >&2
"$O/probe" "$B/sub-range.private-output/2-dry.private.txt" "$O/end.f32" 2>"$O/tail.txt"
OPENBLAS_NUM_THREADS=1 python3 "$D/score.py" "$O" "$B" "$(dirname "$D")/sj6-joined-dry"
