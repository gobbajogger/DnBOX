# Current SuperJ6 Normal prefilter, preparation, and live filter switch

Reference: official Mac 0.999.8 build 9, pinned current original ARM64 slice
`6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850`.
The numeric receipts come from the authorized current original receiver replay
and the independent typed firmware diagnostic; the installed AUv3 inner state
was not instrumented. The separate installed-AU compressor-level gap remains
open. No shipping configuration was changed.

## Two different Normal cases, not a single cutoff recipe

- The earlier waveform fixture is an intentionally projected tracking case:
  physical 32 (Vintage Filter ADSR) = 0 and 39 = 1. Its generated carrier
  wave phase aligns before Normal; its original Normal-only h1 transfer was
  -4.34493 degrees on A3 and -3.60464 degrees on A4. The early matching
  coefficients are valid for this fixture, not for the installed Jupiter cue.
- The **current constructor** sets physical 32 = 1. Public #131 tracking = 0
  writes physical 39 = 0 to all owners. On the cutoff-open, zero-strength
  tracking-free cue, raw #7 = 1 writes physical 36 = 20000.001953125; original
  generator `.37fdbc` receives exactly 20,000 Hz and damping 1 on held
  A3/C4/A4/A5. Actual held 36 edits to 300 Hz at frame 208 and back at 464
  take effect at the next generator entry. Observed callback lengths
  128/17/63/128 and generator calls 8/2/4/8 per voice/channel.
- General tracking is **not** the old linear blend. For example, physical
  39 = .25 in that current branch gives A4 approximately 20019.335243 Hz
  open and 505.142227 from a 300 Hz write. Do not infer an arbitrary formula
  or use capture tables as the firmware law.

The shared Normal coefficient helper (see
`sj6-normal-arithmetic-leaf/normal_coeff.h`) is bit-exact with the current
generator *given the same cutoff and damping*: 3,000 coefficient words, four
rates 32/44.1/48/96 kHz, 15 cutoff values and 10 damping/clamp values.
Coefficient receipt SHA256
`6d6af39487d015ee8dd664a54e687ba0b2392861cf8d7c802d2d56b226077ef5`.
The 2,048-entry original Float32 sine LUT word checksum is
`59767b5065d10fd6b1c82b40088951e0d578283d9b21ae54df53b912c04fcc16`.

At 44.1 kHz with damping 1, coefficients in [b0,b1,b2,a1,a2] order:

- 20 kHz: `0x1.851d9c7caa4aap-1, 0x1.851d9c7caa4aap+0, 0x1.851d9c7caa4aap-1, 0x1.7cb2883c57ccap+0, 0x1.1b116179f9916p-1` (SHA `d23826c36ed90b215c109f2571ae35b41bfba29a57437f76f68adaec5dd81179`).
- 300 Hz: `0x1.cb5f067432c00p-12, 0x1.cb5f067432c00p-11, 0x1.cb5f067432c00p-12, -0x1.ea91fe81f06abp+0, 0x1.d609ac871aeedp-1` (SHA `3377e137a92cdd5dce49b03213909cedb48633d54938e09eabf766f004d5008e`).

The original is direct-form I with a chain of Float64 fused multiply-adds
and narrows each output to Float32 before feedback. Plain Float32 target
arithmetic avoids software FMA calls and performed well in the held wave
fixture, but the valid low-cut/high-resonance envelope and full integrated
audio timing are not passed by that evidence. Preserve the four history
values over coefficient-only changes.

## Live Normal / OTA switch

Public #209 queues physical 51 for Sub/OSC1/OSC2/Noise on actual mode
changes; identical values make no write. Original does **not** change
active mode immediately and does **not** latch it until a new note.
The receiver fades its *summed owner output* toward zero when requested
and active mode disagree, stepping by the Float32 equivalent of
`1/max(1, Fs*.005f)` per sample. At the next at-most-16-frame generation
boundary after gain reaches zero, it selects the requested mode and resets
the newly selected filter states, then ramps gain back toward one. An
opposite edit before commit recovers the original mode without clearing
history. There is no period requiring parallel old/new filter states.
Observed no-extra-note 44.1 kHz transitions 224/225 frames and 48 kHz
256/246, uniform/irregular callbacks respectively. Both directions and
canceled/reasserted controls were tested. The compact immediate global
clear is incompatible. Four-float state reuse is plausible only if this
actual delayed transaction, reset, and output ordering survive integration.
