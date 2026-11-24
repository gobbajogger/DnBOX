# Synth One oscillator coarse controls, Init-only host handoff

Apply six app-only files to the exact checked source in `controls-init-only/source`.
The former path has no exposed 3/4 edit endpoint: observed exact red.
See PREIMAGE/POSTIMAGE ledgers and zero-fuzz application receipt.

- Pinned original AudioKit `DSP/S1Parameter.h`: IDs 3 and 4
  `morph1SemitoneOffset` and `morph2SemitoneOffset`.
  Original `DSP/Kernel/S1DSPKernel.hpp:359–360`: physical -12..+24
  semitones, default 0. `portable_voice.cpp:176–195`: independently
  truncates each semitone control to integer, then scales its own frequency
  with the equal-tempered factor. New integer hardware detents are faithful
  to original audible values; no claim about the exact iOS panel widgets,
  numeric display style, or full original waveform/cadence parity.
- Actual SMK K2/K3 on existing VOICE page, with independent stable IDs and
  explicit +/- bounds. Both existing renderer oscillator pitch branches
  already consume the original IDs; no DSP changes, added page, bank
  provider, arp limit, hardware dependencies or allocation expansion.
- Two independent 6-bit old-invalid q7 MSB planes in verified typed
  Synth One base Sound [8..13]/[14..19]. Code 0 retains authored identity
  and exact value, including general non-Init compatibility; codes 1–37
  are the explicit semitone stops. Preserve the lower seven bits of the
  existing base edits; normal edit preserves coarse MSBs. Raw project/lock
  wire size is unchanged, and note-lock exit restores the live owner.
  Older readers must refuse rather than quietly reopen or erase these
  new projects. Chicago, Miami, SuperJ6 and shared sidecars not changed.
- Init-only real app test 25,005 shift-sanitized assertions: 4,096
  canonical/invalid code combinations, real SMK inputs for both oscillators
  on both parts, factor .5/2/4 measured at applied oscillator pitch,
  sibling oscillator frequency unchanged, audible channel changes with
  other owner's PCM bit-identical, no writes during editing, prior AMP
  value and filter link, actual project cold recall and sequenced lock
  precedence/release. Prior exact parser passes 104 no-write checks against
  both actual newly serialized projects (rejects load and subsequent save,
  current object bytes unchanged). Untouched Init both-owner 4,096 stereo
  frames identical before/after; exact host `original_bass` owner total
  0x2b20 bytes before/after. Coarse target caching is inside the existing
  owner allocation in this host build. Native memory/fit/timing unknown.
- Scope is original 3/4 only. This does not implement ID30 detuning, ID55
  detune LFO, ID56 filter-envelope cadence, or all remaining controls.

Replay: set up private `base` and `source` copies from exact Init-only
preimage. Apply patch to `source` only. On both, append
`const s1c *host_coarse_owner(unsigned o) { return &original_bass[o].dsp.synthone.e; }`
to scratch engine.c only, then run run.sh with this workspace path and
optionally input/output dirs. The trusted echo is a host-test hook only.
Default positive run exports current projects to output; use
`TEST_FLAGS=-DS1_COARSE_OLD=1` and an unpatched scratch input to test rejection,
or `TEST_FLAGS=-DS1_COARSE_RED=1` to verify prior missing editor. Set
`S1_COARSE_PCM` to capture prior/patch untouched PCM instead. All receipts
outside product. No native/device/Codex/Git/remote/package writes.
