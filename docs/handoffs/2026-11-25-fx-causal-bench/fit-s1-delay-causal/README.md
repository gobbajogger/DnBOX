# Why the Synth One delay trial helped Pad but hurt two other presets

Read-only host component analysis. Frozen fitted source and exact six dry
and wet commands as in fit-s1-ab. Both engines also rendered delay-only and
reverb-only for every program: 48 Float32 stereo captures total per A/B variant, no gain
or alignment fit. Use the five *-receipt.txt files for the full 18 scenarios.
Raw captures, presets and executable work remain private.

Every step changes one variable relative to the preceding row:

| Variant | Pad full wet tail | Sahara full wet tail | Atmostrings full wet tail |
|---|---:|---:|---:|
| Exact fitted F16 linear at dd 45 / 18 / 70 | -6.299 | +1.252 | -0.804 |
| Host-only larger F16 ring at dd 17 / 7 / 26 | -2.215 | +2.691 | -0.310 |
| Same retained sample counts but E4M1 linear | -2.392 | +2.647 | -0.305 |
| Switch only right-cascade fractional tap to all-pass | -0.770 | +3.296 | +1.582 |
| Remove only old 1.6x wet boost, keep high rate/right all-pass | -4.857 | +2.802 | +0.512 |

All values are original-relative last-second error (dB); signs denote
too loud/quiet, not absolute signal strength. The first rate-only control
requires 5,720 B here and is not a deployable proposal; the next three
use the same 2,152 B allocation as fitted. Changing F16 to E4M1 at matched
rate/interpolation contributes little to these observed level errors.
Sahara regression largely follows higher bandwidth through its shorter
delay (legacy wet trim remains), while Atmostrings had a much bigger
error change from the right all-pass tap itself at fixed codec and rate.

Isolated effect cases distinguish real gains from cancellation. Rejected
right-allpass Pad delay-only last second is +4.002 dB vs original even
when its full wet looks close. Removing the old trim brings standalone
delay to -0.081 dB, but combined Pad is still short at -4.857 dB. Recovered
spectral energy alone did not restore correct combined phase, feedback
and reverb-input interaction. Sahara delay-only +2.464 to +0.401 dB with
trim removal; full effect remains too loud, and unmodified reverb-only
already measured +1.710 dB. Atmostrings delay-only moves -0.222 to -1.689
as full wet improves from +1.582 to +0.512; the opposing sign is direct
evidence that combined output level is not the per-effect target. Delay
and reverb interact nonlinearly, so don't subtract their dB errors.

All 12 compact fixture dry and reverb-only comparisons (6x2) are
bit-for-bit invariant between exact and E4M1-right. Wet and effect-null
comparisons remain far from exact. No alternative passed the full matrix.

Reproduction: run capture.py and summarize.py as in fit-s1-ab with each
separately built compact receiver. For the wide F16-only arm synth compile
uses exact guarded-fit-near-bound/tree plus this compact.c probe and
environment WIDE_F16=1; the probe substitutes ring storage after patch
selection, without changing any other arena. For E4M1 keep the original
compact-std.c harness (or leave WIDE_F16 unset) and rebuild the scratch
DSP through fit-s1-delay-new/apply.py then six.py, using modes f8,
f8-allpass-right, or f8-allpass-right-nowet respectively. All builds retain
the same stock authorized private FDATA and feature flags. No production,
link, hardware, or Git actions.
