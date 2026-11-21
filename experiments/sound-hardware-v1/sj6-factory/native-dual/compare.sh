#!/bin/sh
# Compare the two renderers on a little-endian Linux host.
set -eu
shipping=$(realpath "$1")
compact=$(realpath "$2")
dual=$(realpath "$3")
mkdir "$4"
out=$(realpath "$4")
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
for variant in shipping dual
do
    if [ "$variant" = shipping ]; then tree=$shipping; else tree=$dual; fi
    mkdir "$out/$variant"
    (cd "$tree" && sh host/build_host.sh --generate-only)
    sh "$here/../native-host/build_runtime.sh" "$tree" "$out/$variant"
done
for program in 0 1 2 3
do
    cmp "$out/shipping/preset-$program.f32le" "$out/dual/preset-$program.f32le"
    sha256sum "$out/dual/preset-$program.f32le"
done
for variant in compact dual
do
    if [ "$variant" = compact ]; then tree=$compact; else tree=$dual; fi
    dsp=$tree/firmware/src/dsp
    ${CC:-cc} -std=c99 -O2 -DDNBX_DIAGNOSTIC_COUNTERS=0 -I"$dsp" \
        "$here/render_typed.c" "$dsp/juno6.c" "$dsp/dnbx_core.c" -lm -o "$out/typed-$variant"
    "$out/typed-$variant" "$out/typed-$variant.f32le"
done
cmp "$out/typed-compact.f32le" "$out/typed-dual.f32le"
sha256sum "$out/typed-dual.f32le"
dsp=$dual/firmware/src/dsp
for source in "$here/admission.c" "$dual/tests/host/juno6_test.c"
do
    ${CC:-cc} -std=c99 -O2 -Wall -Wextra -Werror -DDNBX_DIAGNOSTIC_COUNTERS=0 -I"$dsp" \
        "$source" "$dsp/juno6.c" "$dsp/dnbx_core.c" -lm -o "$out/test"
    "$out/test"
done
sh "$dual/tests/host/juno6_runtime_test.sh" "$out/runtime"
for variant in shipping dual
do
    if [ "$variant" = shipping ]; then tree=$shipping; else tree=$dual; fi
    ${CC:-cc} -std=c99 -O2 -fsanitize=undefined -fno-sanitize-recover=all \
        -DHOST_CHECK -I"$tree/firmware/src/dsp" "$here/break_words.c" -o "$out/break-$variant"
    "$out/break-$variant" >"$out/break-$variant.bin"
done
cmp "$out/break-shipping.bin" "$out/break-dual.bin"
sha256sum "$out/break-dual.bin"
