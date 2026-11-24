# Synth One oscillator 2 additive detuning, Init-only

Six app-only files; apply to exact `controls-init-only/source-s1-coarse`
with the recorded SHA256 preimages. VOICE K4 on existing page exposes stable
original ID 30, distinct from coarse IDs3/4, global multiplier 31 and LFO
route 55. Source: pinned original `DSP/S1Parameter.h` and
`DSP/Kernel/S1DSPKernel.hpp:386` admit continuous -4..+4 (default 0,
original generic unit). Original `portable_voice.cpp:188–206` adds
`DET2 * cached oscillator frequency / 261.6255653006`, then clamps at
Nyquist; LFO route scales that additive term if enabled. Compact chooses
81 manual 0.1 Hz@C4 detents including exact neutral, not a claim of original
UI detent granularity, continuous transfer, or matching realtime slew.
This assigns an existing DSP target and changes no DSP, modulation/cadence,
ARP, stored bank provider, PSRAM, or other output paths. Only settled ID55-OFF
law is qualified here; no new route.

Persistence: old-invalid bits 1–7 of the validated **S1** base Sound marker
byte31 store 0 absent (preserve the exact authored target), 1–81 explicit
(-4.0…+4.0). Former bit0 remains the actual bitcrusher edit marker, and
editing Crush retains all higher bits. Peer, coarse tuning planes, filter
link and note-lock/project wire lengths unchanged. SuperJ6 schema unchanged.
Malformed values fail early. No reader grandfathering or modifying old bytes.

Focused shift-sanitized whole-app test uses real SMK knob CC input for both
owners. Checks codec including malformed domain, pitch C4/C5, both polarities,
sibling oscillator unchanged and selected audio differs with peer PCM exact.
After stock smoothing settled, selected positive C4/C5 increments observed
+3.999237 / +7.998474 Hz against +4/+8 nominal. Initial 7,168-sample transient
was +2.702209 Hz at C4, retained (not original temporal claim). Verifies
Coarse, Crush, filter link, cold project reload, actual sequenced lock and
restore. Exact prior parser refuses new files for both owners and attempted
save without changing any of six persisted objects (104 checks).
Untouched Init 4,096 stereo frames byte-identical before/after; measured
host owner total remains 0x2b20 bytes. No native XIP, fit, timing or release
statement.

Replay as previous coarse handoff: stage a private preimage once, apply patch
there, append host-only accessor
`const s1c *host_coarse_owner(unsigned o) { return &original_bass[o].dsp.synthone.e; }`
to private engine.c for pitch instrumentation. sh run.sh [W [SOURCE [OUT]]].
Raw preimage parser with TEST_FLAGS=-DS1_DETUNE_OLD=1 uses actual exports from
positive OUT; previous instrumented preimage can use TEST_FLAGS=-DS1_DETUNE_RED=1
and S1_DETUNE_PCM for prior PCM before launching binary without PCM env to
see expected missing endpoint. No device, Git, packaging, or production write.
