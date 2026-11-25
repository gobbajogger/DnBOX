# Synth One oscillator 2 Detune LFO (ID55), Init-only

Five app-only files against exact `controls-init-only/source-ui-fx`,
`UI-FX-POST.sha256` 2570e801f697b8587f25ec8536f8c08bb9ca7c147cec73128129b0e0371c8340;
all manifest entries verified on input. No source to remote, package,
device, native link, scope creep to banks, or global cadence changes.

The pinned AudioKit `DSP/S1Parameter.h` and `S1DSPKernel.hpp:411`
establish original ID55 OFF/LFO1/LFO2/BOTH, default OFF. Original
`portable_voice.cpp:188–205` scales only the separately selected additive
OSC2 detune component ID30 by rising LFO1/LFO2/arithmetic mean, unity
OFF (MIDI/key ratio separately applied). The current compact engine
already consumes those exact stable IDs and selector algebra; it has
its own discrete control cadence and smoothing. This exposes the tested
law, not UI-panel equivalence or exact original waveform/cadence.

Real typed PITCH MOD K1 directly before ARPEGGIATOR; no changes to existing
indices of title IDs, prior FX switches, selection ownership, or table
capacity. Persist in previously invalid q7 MSBs [3,4,5] with the existing
typed mod mark. Code zero uses exact authored target; 1–4 are explicit
states. Prior byte30 FM/Decay and Delay/Reverb group remains numerically
untouched and independent: mark iff either group is present. Preserve
low seven level/depth bits on writes and ignore upper bits only for
these q7 readers. Historical slot-2 cannot adopt an explicit route
(the typed high mask is incompatible), and no wire extents change.

Final host 30,924 assertions: actual physical SMK selection and output
both parts, OSC2 delta 3.99924/3.99847/2.23434/3.11639 Hz at C4 for one
asymmetric LFO configuration with detune +4 (both owners), negative
zero-detune exact PCM, peer exact, exhaustive previous-byte30/new-route/
mark combinations, preserving Edit/FM/Decay/FX, project reload, active
note-lock apply and unlock. Untouched Init both-owner 4,096 stereo frames
bit-exact; host pair owner allocation remains 0x2b20. Prior reader exact
real projects reject load/save 104 checks; exact previous endpoint red.
No native fit or timing claim.

Autopan is intentionally not bundled. Original 35/36 uses distinct
amount and tempo-dependent rate (not global requested BPM); compact PAN
amount can change physical stereo delay topology. Safe wire entropy for
both useful rate/amount domains has not been established; no repurposed
FC delays or preset ordinals were claimed.

Replay in private scratch with exact preimage, apply patch zero fuzz,
append test-only accessor
`const s1c *host_coarse_owner(unsigned o) { return &original_bass[o].dsp.synthone.e; }`
to scratch engine.c. Use `sh run.sh [W [SOURCE [OUT]]]`; previous parser
`TEST_FLAGS=-DS1_DT_LFO_OLD=1` (loads actual recorded exports), endpoint
red `TEST_FLAGS=-DS1_DT_LFO_RED=1` and untouched Init dump
`S1_DT_LFO_PCM=FILE`. Tests and evidence only in this packet.
