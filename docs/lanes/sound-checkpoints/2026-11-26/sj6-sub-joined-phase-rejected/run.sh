#!/usr/bin/env bash
set -euo pipefail
B=$PWD/sj6-cross-preset; S=$PWD/sj6-sub-suffix; D=$PWD/sj6-joined-dry; T=$PWD/sj6-hp-exact-exp-fit/kept-elite-1
O=$(mktemp -d); trap 'rm -rf "$O"' EXIT
C=/tmp/sj6-cross-range.c
echo "fd43641d63a755ece2b7446fcbf463b2a8b8dd7f3d28df72ef9dcece46c29e9e  $C" |sha256sum -c >&2
python3 sj6-mixer-diagnostic/variant.py "$C" "$O/center.c" center
python3 "$S/route.py" "$T/firmware/src/app/original_runtime.c" "$O/suffix.c"
for f in suffix "$T/firmware/src/app/original_runtime"; do
 python3 "$B/meter.py" "${f/#suffix/$O/suffix}.c" "$O/$(basename "$f")-meter.c"
done
python3 "$D/instrument.py" "$T/firmware/src/dsp/juno6.c" "$S/compact.c" "$O"
for p in base center joined; do
 if [ "$p" = base ]; then core="$C"; else core="$O/center.c"; fi
 if [ "$p" = joined ]; then rt="$O/suffix-meter.c"; else rt="$O/original_runtime-meter.c"; fi
 SJ6_DNBX_CORE_OVERRIDE="$core" SJ6_RUNTIME_OVERRIDE="$rt" SJ6_RENDER_SOURCE="$O/probe.c" \
 SJ6_EXTRA_FLAGS="-I$S" SJ6_JUNO_OVERRIDE="$O/juno-measured.c" \
 bash "$D/build.sh" "$T" "$O/$p" >&2
 for f in "$B"/sub-range.private-output/*.private.txt; do
   key=$(basename "$f" .private.txt); "$O/$p" "$f" "$O/$p-$key.f32" 2>"$O/$p-$key.txt"
 done
 for effect in bypass classic classic-legacy; do
   for n in 1 6; do
    SJ6_STRESS="$effect:$n" "$O/$p" "$B/sub-range.private-output/2-dry.private.txt" "$O/$p-$effect-$n.f32" 2>"$O/$p-$effect-$n.txt"
   done
 done
done
OPENBLAS_NUM_THREADS=1 python3 "$D/score.py" "$O" "$B"
