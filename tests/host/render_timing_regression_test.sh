#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Short per-block render timing regression (perf lane): a few seconds of simulated audio, not a soak.
# Renders every engine through the production engine_render path (two 256-frame stretches per
# 512-frame half, as the ALNK0 ISR does): Chicago+Miami and Synth One+SuperJ6 synth parts, with
# drum909, drum808, BREAK and the FX bus all busy (tests/host/audio_stress_test.c workload).
# original_runtime.c is compiled at the firmware's level (tools/build.py ORIGINAL_SYNTH_OZ_UNITS).
# Checks (tools/render_timing_check.py): 0 late blocks against the 11,609 us half deadline divided
# by a safety factor, and per-engine mean / block max CPU time, normalised by the drum/FX/BREAK
# rest of the same block, within a tolerance of tests/host/render_timing_baseline.json.
# Host timings are not AC79 timings; thread CPU time keeps it stable on a loaded box.
set -eu
cd "$(dirname "$0")/../.."
OUT=${X0X_TIMING_OUT:-build/host/render-timing}
BLOCKS=${X0X_TIMING_BLOCKS:-256}
REPS=${X0X_TIMING_REPS:-2}
# A shared runner may have a stable core reserved for the timing probe.
# Pin only the measured process when requested; keep all samples and limits.
run_stress() {
    if [ -n "${X0X_TIMING_CPU:-}" ]; then
        case "$X0X_TIMING_CPU" in *[!0-9]*) echo "invalid X0X_TIMING_CPU" >&2; return 2 ;; esac
        taskset -c "$X0X_TIMING_CPU" "$@"
    else
        "$@"
    fi
}
mkdir -p "$OUT"
awk '/^static void smk_wheels\(void\)/{on=1} on{print} on && /^}/{exit}' firmware/src/app/main_fm1.c > "$OUT/smk_wheels.inc"
test -s "$OUT/smk_wheels.inc"
. tests/host/original_build.sh
ORIGINAL_FLAGS="$ORIGINAL_FLAGS -DX0X_FAST_FMA"
sh host/build_host.sh --generate-only >"$OUT/assets.log" 2>&1
RT_OPT=${X0X_TIMING_RUNTIME_OPT:-$(python3 -c "
import importlib.util
s = importlib.util.spec_from_file_location('build', 'tools/build.py'); b = importlib.util.module_from_spec(s); s.loader.exec_module(b)
print('Oz' if 'original_runtime' in b.ORIGINAL_SYNTH_OZ_UNITS else 'O2')")}
compile() {
    ${CC:-cc} -g -std=c99 -Wno-unused-function -Wno-unused-parameter -DX0X_HOST $ORIGINAL_FLAGS \
        -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen -I"$OUT" "$@"
}
renames=
for entry in prepare key param init render release_source cut_source all_off retire; do
    renames="$renames -Doriginal_runtime_$entry=stress_real_original_runtime_$entry"
done
compile -$RT_OPT $renames -c firmware/src/app/original_runtime.c -o "$OUT/runtime.o"
sources=
for source in $ORIGINAL_SOURCES; do
    case "$source" in firmware/src/app/original_runtime.c) ;; *) sources="$sources $source" ;; esac
done
compile -O2 tests/host/audio_stress_test.c $sources "$OUT/runtime.o" -lm -o "$OUT/stress"
csvs=
for pair in 0,1 2,3; do
    case "$pair" in 0,1) factory=18; choice=6 ;; *) factory=0; choice=0 ;; esac
    rep=1
    while [ "$rep" -le "$REPS" ]; do
        csv="$OUT/${pair%,*}${pair#*,}-$rep.csv"
        X0X_STRESS_KINDS=$pair X0X_STRESS_DEADLINE_NS=0 run_stress "$OUT/stress" "$csv" /dev/null "$BLOCKS" "$factory" "$choice" > "$OUT/run.log" 2>&1 \
            || { cat "$OUT/run.log"; echo "FAIL: stress run pair=$pair rep=$rep"; exit 1; }
        csvs="$csvs $pair:$csv"
        rep=$((rep + 1))
    done
done
python3 tools/render_timing_check.py --runtime-opt "$RT_OPT" --baseline tests/host/render_timing_baseline.json $csvs
