#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd);R=$(dirname "$D")
B="$R/sj6-cross-preset";T="$R/sj6-hp-exact-exp-fit/kept-elite-1"
export TMPDIR="$D"; O=$(mktemp -d);trap 'rm -rf "$O"' EXIT
cd "$R"
echo "fd43641d63a755ece2b7446fcbf463b2a8b8dd7f3d28df72ef9dcece46c29e9e  /tmp/sj6-cross-range.c"|sha256sum -c >&2
python3 "$R/sj6-raw-j60-common/tap.py" "$T/firmware/src/app/original_runtime.c" "$B/render.c" "$O"
mkdir -p "$O/firmware/src/dsp"
cp "$T/firmware/src/dsp/juno6_j60_native.inc" "$O/firmware/src/dsp/"
patch -d "$O" -s --fuzz=0 -p1 <"$R/sj6-j60-wet-native/experimental.patch"
python3 "$D/generate.py" "$O/firmware/src/dsp/juno6_j60_native.inc" "$T/firmware/src/dsp/juno6.c" "$O"
sed 's@firmware/src/dsp/juno6.c "\${SJ6_DNBX_CORE_OVERRIDE@"\${SJ6_JUNO_OVERRIDE:-firmware/src/dsp/juno6.c}" "\${SJ6_DNBX_CORE_OVERRIDE@' "$B/build.sh" >"$O/build"
# Preserve actual build script filesystem-relative donor paths without modifying it.
sed -i 's@D=$(realpath "$(dirname "$0")")@D="'"$B"'"@' "$O/build"
grep -q 'SJ6_JUNO_OVERRIDE' "$O/build"
for M in 0 1; do
 export SJ6_EXTRA_FLAGS="-DDNBX_EXPERIMENT_J60_POST=$M" SJ6_JUNO_OVERRIDE="$O/juno6.c" SJ6_RENDER_SOURCE="$O/driver.c" SJ6_RUNTIME_OVERRIDE="$O/runtime.c" SJ6_DNBX_CORE_OVERRIDE=/tmp/sj6-cross-range.c
 bash "$O/build" "$T" "$O/probe-$M" >&2
 for A in dry authored;do
   "$O/probe-$M" "$B/sub-range.private-output/2-$A.private.txt" "$O/$M-$A.f32" 2>"$O/$M-$A.log"
   mv "$O/tap-0.f32" "$O/$M-$A-in.f32"
   mv "$O/tap-1.f32" "$O/$M-$A-out.f32"
   mv "$O/blocks.i32" "$O/$M-$A-blocks.i32"
 done
done
python3 "$D/score.py" "$O" "$B"
