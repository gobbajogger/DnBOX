#!/usr/bin/env bash
set -euo pipefail
D=$PWD/sj6-raw-j60-common B=$PWD/sj6-cross-preset T=$PWD/sj6-hp-exact-exp-fit/kept-elite-1
O=$(mktemp -d); trap 'rm -rf "$O"' EXIT
C=/tmp/sj6-cross-range.c
echo "fd43641d63a755ece2b7446fcbf463b2a8b8dd7f3d28df72ef9dcece46c29e9e  $C" |sha256sum -c >&2
python3 "$D/tap.py" "$T/firmware/src/app/original_runtime.c" "$B/render.c" "$O"
SJ6_RENDER_SOURCE="$O/driver.c" SJ6_RUNTIME_OVERRIDE="$O/runtime.c" SJ6_DNBX_CORE_OVERRIDE="$C" bash "$B/build.sh" "$T" "$O/probe" >&2
"$O/probe" "$B/sub-range.private-output/2-authored.private.txt" "$O/typed-out.f32" 2>"$O/route.log"
export OPENBLAS_NUM_THREADS=1 PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench SJ6_BIN="$PWD/ab-effects-readonly/SJ6-AU.private"
python3 "$D/score.py" "$O" "$B"
