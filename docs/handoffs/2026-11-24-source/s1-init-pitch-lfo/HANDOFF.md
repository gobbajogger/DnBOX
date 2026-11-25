# Synth One global Pitch LFO (ID57), Init-only

Five app-only files against exact `controls-init-only/source-ui-pitchmod`
(`PITCH-MOD-POST.sha256`, cb20b1ab prefix, verified entry by entry).
No presets, native, device, publication, Git or high-impact writes.

Pinned original `S1Parameter.h` and `S1DSPKernel.hpp:413` identify ID57
OFF/LFO1/LFO2/BOTH default OFF. `portable_voice.cpp:153–160,176–223`
uses `1 + ascending(LFO)*(2^(1/12)-1)` as multiplier of the oscillator
1/2 base, sub and FM frequencies. OFF is unity; BOTH is arithmetic mean.
ID30's additive OSC2 detune is separately added, as before; original
pitch bend/down-up wheel domain, tune, and ID55 detune-only LFO are not
aliased. The compact renderer already does the same scalar pitch math
on its existing ticks, not proof of original per-frame cadence or PCM.

PITCH MOD K2 (K1 still ID55), no page or title changes. Add only formerly
invalid historic high ordinal [21] bits3–5, witnessed by existing typed
mod mark; 0 absent preserves exact program word, 1–4 explicit,
5–7 invalid. Exhaustive prior ordinals 0–694 preserve exact meaning.
Do not allocate byte7 or [21] bit7, which is independently reserved for
future ARP. Earlier byte30 FX/mod bits and q7 ID55 untouched; mark held
if either old or new independent payload is present. No extent changes.

55,709 shift-sanitized actual host checks: both owners use real SMK K2
input; actual primary oscillator C4 261.57596 Hz OFF, 277.12704 at LFO1,
270.26596 at LFO2, 273.69650 at BOTH under the asymmetric witness.
Per-tick OSC2/SUB/FM match separate scaled expectations; OSC2 additive
ID30 remains 3.99924 Hz across four modes. Both original null witnesses
zero LFO depths with nonzero detune are exact whole stereo.
Selected nonzero audio changes with opposite owner bit-identical.
Cold save/load, actual note-lock capture, edit refusal and unlock,
earlier physical ID55/FM/decay/FX/level/depth preserved. Exact old parser
refuses new projects and subsequent save without writes (104 checks).
4,096 untouched Init stereo frames byte-exact. Pair host owner aggregate
remains 0x2b20. Native fit and original time-domain waveform not measured.

Replay: copy preimage in private scratch once, apply patch with
`--no-backup-if-mismatch --fuzz=0`, append test-only accessor
`const s1c *host_coarse_owner(unsigned o) { return &original_bass[o].dsp.synthone.e; }`
to scratch engine.c. Run `sh run.sh [W [SOURCE [OUT]]]`; old parser
`TEST_FLAGS=-DS1_P_LFO_OLD=1`, prior missing endpoint
`TEST_FLAGS=-DS1_P_LFO_RED=1`, optional untouched dump
`S1_P_LFO_PCM=FILE`. Only test source and aggregate receipts here.
