# SPDX-License-Identifier: GPL-3.0-only
# 808 scratch inside the out_l/out_r window must be bit-identical to the separate
# t0/tr buffers of BASE_REF: split windows, deferred and failed CPU2, ASan/UBSan; a partially
# finished CPU2 job must give the clean deferred output.
# Needs the private original-synth inputs and sh host/build_host.sh --generate-only.
set -e
cd "$(dirname "$0")/../.."
python3 tests/test_archive_engine_ui_compat.py
python3 tests/test_archive_engine_runner.py
# Preserve the executable exit status before displaying its final output line.
run_audio_checked() {
 log=$1; pcm=$2; blocks=$3; bytes=$4; shift 4
 if ASAN_OPTIONS=detect_leaks=0 "$@" "$pcm" > "$log"; then
  if ! grep -Fq "PASS engine output windows: $blocks blocks;" "$log"; then
   echo "missing complete audio marker" >&2; return 1
  fi
  if [ ! -f "$pcm" ] || ! actual=$(wc -c < "$pcm"); then
   echo "missing readable audio bytes" >&2; return 1
  fi
  if [ "$actual" -ne "$bytes" ]; then
   echo "incomplete audio bytes" >&2; return 1
  fi
  tail -1 "$log"
 else
  rc=$?
  cat "$log"
  return "$rc"
 fi
}
D=${OUT:-build/host}/engine-808-alias; rm -rf $D; mkdir -p $D; BASE=${BASE_REF:-refs/tags/archive/build/0.0.22}
git show $BASE:firmware/src/app/engine.c > $D/engine-base.c
# The archived engine predates the current UI's optional BREAK audition API.
# Its comparison exercises render/CQ scratch only, never sample browsing. Refuse
# audition in that baseline rather than changing the archived audio implementation.
cat >> "$D/engine-base.c" <<'BASE_PREVIEW_COMPAT'
int engine_brk_preview(int slot) { (void)slot; return 0; }
void engine_brk_preview_end(void) { }
int engine_brk_preview_active(void) { return 0; }
/* Current UI-only snapshot accessors are not exercised by the archived audio
 * comparison. Report unavailable, never synthesize envelope or waveform state.
 * These definitions are appended only to engine-base.c; current engine APIs
 * and the render/CQ comparison path remain unchanged. */
uint32_t engine_brk_outline_revision(int which) { (void)which; return 0; }
int engine_brk_outline_snapshot(int which, uint8_t *peaks, int n, uint32_t *revision)
{ (void)which; (void)peaks; (void)n; (void)revision; return 0; }
int engine_drum808_curve(int voice, drum_curve_t *out)
{ (void)voice; (void)out; return 0; }
int engine_drum808_param_curve(int voice,int param,drum_curve_t *out)
{ (void)voice; (void)param; (void)out; return 0; }
int engine_drum808_param_curve_preview(int voice,int param,int selection,int raw,drum_curve_t *out)
{ (void)voice; (void)param; (void)selection; (void)raw; (void)out; return 0; }
int engine_bd909_pitch_view(float *ms, float *hz)
{ (void)ms; (void)hz; return 0; }
int engine_drum909_decay_view(int voice, engine_decay_view_t *out)
{ (void)voice; (void)out; return 0; }
int engine_drum909_amp_view(int voice, drum909_amp_snapshot *out)
{ (void)voice; (void)out; return 0; }
int engine_juno_envelope_view(int voice, juno6_envelope_snapshot *out)
{ (void)voice; (void)out; return 0; }
/* The archived renderer has no fire-timestamp UI observation or arp state.
 * The shared preset include calls this hook even in an arp-off render build. */
int engine_drum_fired(int kit, int voice, int step, uint32_t window_ms)
{ (void)kit; (void)voice; (void)step; (void)window_ms; return 0; }
static void arp_adopt(int voice, const sound_t *sound, int preset_change)
{ (void)voice; (void)sound; (void)preset_change; }
BASE_PREVIEW_COMPAT
U=$(python3 - <<'PY'
import importlib.util
spec=importlib.util.spec_from_file_location('b','tools/build.py');b=importlib.util.module_from_spec(spec);spec.loader.exec_module(b)
b.synth_build_config();b.require_original_synth_sources()
print(' '.join(['firmware/src/dsp/drum909.c','firmware/src/dsp/drum808.c','firmware/src/dsp/x0x_param.c','firmware/src/dsp/breaks.c','firmware/src/dsp/fxbus.c','firmware/src/dsp/master.c','firmware/src/seq/sequencer.c','firmware/src/seq/tb3po.c','firmware/src/seq/pattern.c','firmware/src/seq/motion.c','firmware/src/app/factory_banks.c',*b.ORIGINAL_SYNTH_UNITS,*b.ORIGINAL_SYNTH_BACKENDS]))
PY
)
# Match the archive configuration: arp is independently covered by the
# production-on gate and does not belong to this archived 808 A/B.
for mode in normal san; do
 for dw in 1 0; do
 F="-O1 -g -ffp-contract=off -std=c99 -w -DX0X_HOST -DX0X_ARP_HOOKS=0 -DX0X_ORIGINAL_SYNTH=1 -DX0X_WOBBLE=0 -DX0X_DEV_WOBBLE=$dw -fno-fast-math -DORIGINAL_USE_SOFT_FMA -DMIAMI_FX_NO_HOST_FENV -D_POSIX_C_SOURCE=200809L -Iexperiments/original_synth -Iexperiments/wobble -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen"
 [ $mode = san ] && F="$F -fsanitize=address,undefined -fno-sanitize-recover=all"
 for v in base new; do
  src=$D/engine-base.c; [ $v = new ] && src=firmware/src/app/engine.c
  cc $F -Dseq_until_event=output_window_until_event -Dplat_cpu2_run=output_window_cpu2_run -Dplat_cpu2_wait=output_window_cpu2_wait -c $src -o $D/eng-$v.o
  cc $F -DOW_LO=0 -DOW_HI=3 tests/host/engine_output_window_test.c $U $D/eng-$v.o -lm -o $D/h-$v
  run_audio_checked "$D/$mode-$dw-$v.log" "$D/$mode-$dw-$v.pcm" 600 591360 "$D/h-$v"
 done
 cmp $D/$mode-$dw-base.pcm $D/$mode-$dw-new.pcm
 echo "BITEXACT $mode DEV_WOBBLE=$dw $(stat -c%s $D/$mode-$dw-new.pcm) B $(sha256sum < $D/$mode-$dw-new.pcm | cut -c1-16)"
 # a core-1 job that failed half done: the output equals this engine's clean deferred run
 # (the old engine rendered the 808 and BREAK twice there: not a reference)
 for r in 3 1; do
  cc $F -DOW_LO=$r -DOW_HI=$((r+1)) tests/host/engine_output_window_test.c $U $D/eng-new.o -lm -o $D/h-m$r
  run_audio_checked "$D/$mode-$dw-m$r.log" "$D/$mode-$dw-m$r.pcm" 200 197120 "$D/h-m$r"
 done
 cmp $D/$mode-$dw-m3.pcm $D/$mode-$dw-m1.pcm
 echo "PARTIAL=CLEAN $mode DEV_WOBBLE=$dw"
 done
done
