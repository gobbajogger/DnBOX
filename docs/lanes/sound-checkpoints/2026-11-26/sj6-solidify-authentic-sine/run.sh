#!/usr/bin/env bash
set -euo pipefail
D=$PWD/sj6-solidify-authentic-sine B=$PWD/sj6-cross-preset T=$PWD/sj6-hp-exact-exp-fit/kept-elite-1
O=$(mktemp -d); trap 'rm -rf "$O"' EXIT
C=/tmp/sj6-cross-range.c
echo "fd43641d63a755ece2b7446fcbf463b2a8b8dd7f3d28df72ef9dcece46c29e9e  $C" |sha256sum -c >&2
python3 sj6-mixer-diagnostic/variant.py "$C" "$O/pinned.c" center
python3 "$D/phase.py" "$O/pinned.c"
python3 "$B/meter.py" "$T/firmware/src/app/original_runtime.c" "$O/runtime.c"
sed 's/int main(int argc,char\*\*argv)/__attribute__((visibility("default"))) int run_render(int argc,char**argv)/' "$B/render.c" > "$O/render.c"
SJ6_DNBX_CORE_OVERRIDE="$O/pinned.c" SJ6_RUNTIME_OVERRIDE="$O/runtime.c" SJ6_RENDER_SOURCE="$O/render.c" SJ6_EXTRA_FLAGS="-DDNBX_HOST_SINE_PHASE=2 -I$PWD/sj6-wave-carrier -shared -fPIC -fvisibility=hidden $D/bank.c" bash "$B/build.sh" "$T" "$O/probe.so" >&2
export OPENBLAS_NUM_THREADS=1
export PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench
python3 "$D/verify.py" "$O/probe.so" > "$D/verify.json"
python3 "$D/score.py" "$O" "$B"
