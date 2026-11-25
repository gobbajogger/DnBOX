# Original source witness for Mono, Legato and Glide Time

Build 9 authorized AU SHA256 a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1, static analysis only.
FXController's actual monoToggle, legatoToggle and glideKnob connect to
Conductor's controls 32/33/35. Authenticated default callback at 0x10001f2a8
indexes the byte jump table and sends 32–35 to the zero-loaded return.
No independent visible Glide Enable switch was inferred.

- Mono parameter observer 0x10026e990 → 0x1000f14f0. It thresholds >0 and
  sends AKSampler parameter 28. At 0x1000f1648 the same normalized input
  is copied unchanged into s0; 0x1000f1650 calls original Glide Enable
  callback 0x1000f1794. Thus the actual visible Mono action also enables
  Glide; turn off disables it. An ID35 rate turn alone does not arm Glide.
- Legato observer 0x10026ea44 → 0x1000f1900 → AKSampler parameter 29.
  0x10036f960 queries existing held notes before adding the accepted new
  note (helper 0x10038e51c checks positive per-pitch counts). At the mono
  branch 0x10036fc50–fc84, positive prior held state plus legato selects
  voice0 retune 0x1003a5208 instead of fresh onset 0x1003a5004.
  The retune writes pitch/wave routing and pitch correction, not the
  amplitude envelope onset or the captured velocity field. At 0x10037016c
  the release fallback also uses the retune branch with Legato; no repeat
  note-off is dispatched for inactive owners. The ordinary nonlegato
  fallback reattacks at literal velocity 100 (0x1003701b0–c4).
  Release selection helper 0x10038e544 finds lowest remaining note.
- Glide Time observer 0x10026ebac→0x1000f1b58 invokes 0..0.5s linear
  mapping with binary64 bounds/exponent then AKSampler rate parameter 9.
  Overlap correction 0x1003a5128–74/0x1003a5314–64 computes prior active
  offset −12×log2(new/previous Hz) and clears |offset|<0.01 semitone.
  At 0x1003a5c20–80 each prepared span moves toward zero by
  12×spanFrames/(sampleRate×timeSeconds), clamped on sign crossing.
  0x1003a5e2c–4c reads the offset into semitone pitch composition.
  Original normal inner prep uses spans up to 16; the host compact
  implementation uses a deterministic sixteen-frame control clock with
  per-frame oscillator/sub increment and preserves that clock across
  caller splits. No arbitrary original host block-boundary or custom
  tuning claim. Equal-tempered nominal difference uses MIDI-note delta.

Implementation scope is compact equal-tempered Mono/Legato and actual
phase change. Existing compact filter shapes/envelopes, partial runtime
peer oscillator and unsupported original live pedal/arming behavior are
not claimed A/B-exact. Hot Mono mode switch is deliberately refused,
and grouped history changing the Mono bit refuses with actual live voice.
