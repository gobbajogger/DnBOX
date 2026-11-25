# Original Synth One ID58 CRUSH LFO

At pinned AudioKit/AudioKitSynthOne 6466a3715c96b7ecf1dd255a218cbd571408a314,
EffectsPanelController.swift binds visible bitcrushLFOToggle to .bitcrushLFO;
S1Parameter.h assigns 58, S1DSPKernel.hpp defines selector 0..3 default OFF,
S1DSPKernel+process.mm computes the live log-rate from effective base Rate
ID34 and current ascending LFO, clamps 2048..48000 Hz, then runs the original
sample-and-hold state. BOTH averages envelopes. It is sample-rate modulation,
not bit-depth, pitch or reverb. Original operator order advances idx with
a binary32 add then subtract (not compact legacy k-rate algebra grouping).

Fenced active validated selections alone use corrected cadence. Rate is
computed at each actual sample capture; intervening rates are unobservable
because original increment is only used at capture. This avoids redoing
log/pow calculations while holding a sample. Standard host real-math calls
avoid the compact FAST approximation only in this branch. No global filter,
delay, LFO, dry, default, headroom or rendering schedule change. Same
boundary compares active probes against independent original source recurrence
and same captured input, not original whole-engine PCM or libm compile
identity on all platforms. Hot-path timing/native fit not checked.

Physical CRUSH MOD appears as one typed-only adjacent FX page with selector
OFF, LFO 1, LFO 2, BOTH; prior MOD wheel and FX including ID51 and RV SIZE
retain their existing bindings. No duplicate crush/sample rate control,
extra aliases or new note-lock extent. b20 host-only owner storage, canonical
0, values 1..3; no use of SuperJ6 DELAY b16–19 or previously claimed bytes.
Actual Undo/Redo, saved/validated recall and read-only previous tuple. No
legacy interpretation, presets or default activation.

Test recipe: sh replay.sh ../controls-init-only/source-mod-wheels
Single private scratch; test tap reads the actual DSP sample boundary,
original capture law observed 4,096 contiguous frames × three distinct
source routes × two owners, each in FAST build. Trace also reproduces stale
KR counterfactual discrepancy. Other proofs rely on real physical UI ingress,
unchanged old reader, and untouched baseline comparison. Native, device, Git,
project assets and release paths not touched.
