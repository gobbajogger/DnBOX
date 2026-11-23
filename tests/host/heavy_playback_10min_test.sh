#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# perf item 2: 10 simulated minutes of all-engine heavy playback on the whole host app, for the
# Elite and the Pro build identity. Every 512-frame block (11,609.98 us of audio at 44.1 kHz) is
# timed with thread CPU time and must finish under 11,609 us; dropouts (a lost MIDI/SMC input or an
# original-runtime fault) must be 0. Two synth parts run at once, so the engines run in pairs:
# Chicago + Miami (Chicago FX 6, Miami factory 18) and Synth One + SuperJ6. Both drum kits play every
# 32nd step, breaks roll, bus FX, reverb, delay and master are all on, 48 MIDI packets per block.
# Host CPU timings are not AC79 timings: this is the repo's simulated measurement, not a device claim.
set -eu
cd "$(dirname "$0")/../.."
OUT=${X0X_HEAVY_OUT:-build/host/heavy-playback}
BLOCKS=${X0X_HEAVY_BLOCKS:-51680}           # 51,680 x 512 / 44,100 = 600.0 s
DEADLINE_NS=${X0X_HEAVY_DEADLINE_NS:-11609000}
mkdir -p "$OUT"
awk '/^static void smk_wheels\(void\)/{on=1} on{print} on && /^}/{exit}' firmware/src/app/main_fm1.c > "$OUT/smk_wheels.inc"
test -s "$OUT/smk_wheels.inc"
. tests/host/original_build.sh
ORIGINAL_FLAGS="$ORIGINAL_FLAGS -DX0X_FAST_FMA"
sh host/build_host.sh --generate-only >"$OUT/assets.log" 2>&1
renames=
for entry in prepare key param init render release_source cut_source all_off retire; do
    renames="$renames -Doriginal_runtime_$entry=stress_real_original_runtime_$entry"
done
sources=
for source in $ORIGINAL_SOURCES; do
    case "$source" in firmware/src/app/original_runtime.c) ;; *) sources="$sources $source" ;; esac
done
fail=0
models=${X0X_HEAVY_MODELS:-Elite Pro}
for model in $models; do
    compile() {
        ${CC:-cc} -O2 -g -std=c99 -Wno-unused-function -Wno-unused-parameter -DX0X_HOST "-DSMK_MODEL=\"SMK-37 $model\"" \
            $ORIGINAL_FLAGS -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen -I"$OUT" "$@"
    }
    compile $renames -c firmware/src/app/original_runtime.c -o "$OUT/runtime-$model.o"
    compile tests/host/audio_stress_test.c $sources "$OUT/runtime-$model.o" -lm -o "$OUT/stress-$model"
done
# The four runs are independent; run them side by side at the lowest priority (timing is thread
# CPU time, so sharing the host does not change a block's measured cost).
pids=
for model in $models; do
    for pair in "0,1 18 6" "2,3 0 0"; do
        set -- $pair
        tag="$model-$(echo "$1" | tr , -)"
        rm -f "$OUT/$tag.rc"
        ( rc=0; X0X_STRESS_KINDS=$1 X0X_STRESS_DEADLINE_NS=$DEADLINE_NS \
            nice -n 19 "$OUT/stress-$model" "$OUT/$tag.csv" /dev/null "$BLOCKS" "$2" "$3" >"$OUT/$tag.log" 2>&1 || rc=$?
          echo "$rc" >"$OUT/$tag.rc" ) &
        pids="$pids $!"
    done
done
for pid in $pids; do wait "$pid" || true; done
for model in $models; do
    for kinds in 0-1 2-3; do
        tag="$model-$kinds"
        if [ "$(cat "$OUT/$tag.rc" 2>/dev/null)" = 0 ]; then
            line=$(grep '^heavy:' "$OUT/$tag.log" || true)
            case "$line" in *" dropouts=0 late=0 "*) echo "PASS $model $line" ;; *) echo "FAIL $model $line"; fail=1 ;; esac
        else
            echo "FAIL $model kinds=$kinds: $(tail -2 "$OUT/$tag.log" | tr '\n' ' ')"; fail=1
        fi
    done
done
secs=$(awk "BEGIN{printf \"%.1f\", $BLOCKS*512/44100}")
[ "$fail" = 0 ] && echo "heavy-playback: PASS ${secs} s x 2 engine pairs x $models, dropouts=0, late=0" || echo "heavy-playback: FAIL"
exit "$fail"
