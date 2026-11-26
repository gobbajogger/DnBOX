# Older Main and Sub handoff causality, exact ARM64 signed original

From workspace root, set SJ6_BIN to authorized ab-effects-readonly/SJ6-AU.private
and PYTHONPATH to the local Unicorn 2.1.4 and ab-effects-readonly/sj6-fullbench/bench.
Run python sj6-owner-causality/probe.py. Original and authored receiver fixture
are pinned. Synthetic deterministic continuous-loop Sub zones witness cursor
state only; no original recordings or app audio output are bundled.

Actual original Main1 and Sub play key36@86 then native receiver chord
48/52/55@86 at relative frame 33075, with first note held or released
11025/22050 samples earlier. Both share exact AMP prep history and cross
at relative 33491 for all tested cases; these are test observations, not
a fixed-delay policy. The Sub's active +0x28 remains distinct from pending
+0x5d8 before crossing, and its double +0x10 sample cursor keeps advancing.
At sample pointer-copy 0x1003a5b30..b6c, original uses incoming asset start
(zero in marker zones) and resets independent saw phase +0x52c and history.

Crucial: key word already 55 during pending, desired frequency about
195.86 Hz, but actual oscillator step ramps with glide while old phase
remains live. In the 11025 lead case the 44.1 kHz phase increment increases
.00148468534 -> .00148623448 -> .00152389961 approaching crossing, i.e.
about 65.475 -> 67.204 Hz (not 195.86, and not constant). Sub cursor advances
16.01669 then 16.03340 samples over corresponding prep intervals, despite
stored base sample ratio staying 2.994558 until the zone swap. The actual
old audio-rate cursor is not the stored base ratio. No per-note offset/gain,
current-AU waveform or firmware fit claims.

The recovered guarded typed Main currently already applies sv glide to
lv->increment before its delayed reset and feeds it to 4x sampler. Its
nearby stock synthetic Sub is not a faithful sampled zone witness; validate
the precise provider that generated any separately authenticated Sub tape.
