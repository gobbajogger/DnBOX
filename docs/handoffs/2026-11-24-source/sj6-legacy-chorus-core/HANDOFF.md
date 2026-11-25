# Implemented legacy AKChorus alternate core, not editor parity

Off-product C, 44.1 kHz. Execute `sh replay.sh`. Product, SMK UI, owner
storage, DSP modes, J60 and previous Chorus preview untouched.

## Original boundaries

Same authenticated Build 9 AU a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1
as sibling `sj6-original-chorus-control`. This is a working reconstruction
of the **alternate core**, from the exact disassembly, not of the aggregate
app Chorus graph. The leaf takes two already-decided independent source
inputs and a separately selected dry/wet state. It does not guess when the
app should activate this core.

- `0x1003ae5b8`: 20 ms ramp; ring ceil(6ms*sr)+4, min 8. Fixed
  44.1 kHz gives 882 frames and 269 Binary64 samples on each side.
- `0x1003ae21c`: independent pre/post transfer coefficients with
  published-in-executable residues/poles and matched-z at max(sr,44100).
  Each bank contains three parallel sections, channel runs its own states.
- `0x1003ae1c0`, `0x1003aeb64`: section state and denormal floor; outputs
  summed then gain-normalized; independently prefilter each input, delay
  and postfilter, and use original triangle/opposed spread plus shared cosine.
- `0x1003aea7c`: cubic 4-sample delay interpolation, clamped to 2..n-3.
- `0x1003ae770`/ `0x1003ae8c4`: separate targets for rate, depth,
  wet gain, mix and mode-dependent shape. Explicit ID42–44 translation
  then effective 0.1..10 Hz bound at alternate AU receiver. No J60 IDs.
- `0x1003ad63c`: AU parameter jump table has older 0–3 (and ramp 4)
  and a separate 7–9 parameter family for rate, depth and wet gain.
  `0x1003add30` selects alternate using wrapper mode = 2, resets during
  engagement and tracks warmup/crossfade; the lower core remains active
  despite zero mix once run. App I/II, app model byte 208 and wrapper
  mode are not interchangeable. The scoped selector adapter is not proof
  that ordinary app edits dispatch to that alternate branch.

## Tests and hold

Actual compiled C receiver uses separate owners. Tests exercise all three
original ID command ranges and independent significantly audible changes,
a left-only impulse leaving right entirely silent, zero-wet dry output
with still-advancing ring and phase, retained phase/peer state across edits,
unsafe/J60 command refusals, and sanitizer execution. Host state 5,184 bytes
per active owner. No change to native pool, no fit or performance claim.

Need authenticated application path to alternate AU mode and its 7–9 controls
and independently captured original hearable reference before promoting a
physical binding. The production preview's classic 256-sample mono ring
cannot supply these signals. Coefficient bit/PCM and complete wrapper
transition parity remain unverified; never count a bare reconstructed core
run as the original standalone/AU app. No alternate fake marked as full
model; no new tentative SMK knobs enabled.

For the later corrected same-tree replay, this directory is an intermediate dependency. Do not promote the old wrapper feedback law.
