# Populated Synth One render: measured optimization

Use the sealed Synth One-only diagnostic, exact width, stock compact
delay, fixed worker plan A, and the six held USB MIDI edges. Use both
stage-trace.patch and voice-admission.patch, and enable their two flags.
The clock and companion payload verification are unchanged. The trace
compares actual voice kernel admission and audio deadlines. Variants
may render different counts because of underruns. Never sum the two
cores' overlapping durations.

The traced baseline peaks at 19.293/19.085 ms, 225/227 underruns. Worker
A takes 15.878/15.751 ms at those worst blocks, while the main core's
known ordered waits take 9.444/6.779 ms. This is now distinct from the
unvoiced case where 909/background costs are largest. The 10.809 ms
target is unchanged.

The guarded keycache patch memoizes exact existing semitone ratios per
owner and exact key-tracking weights per voice, using the original
arguments and float expressions. It costs 184 B of combined pool (8,832
B beyond the stop reserve remain); both binaries boot and play the
voiced cue. One run reaches 18.111/18.003 ms and 224/223 underruns.
The bit-exact host A/B includes six programs, wet and delay arms,
plus held-note edits to both semitone controls. These checks compare the
candidate to old compact output, not the original app's delay.

Using -O2 for only s1c/s1u instead of -Oz (existing strict FP flags)
reaches 16.417/16.612 ms and 211/201 underruns. The combined and hot
inline experiments are separately recorded. Nothing here is production
qualified, and the delay representation was not changed.

The exact hot-inline variant reaches 17.347/17.020 ms and 214/213
underruns. Combining the existing -O2 knobs and the keycache fits
(71,364/71,372 B app flash, 8,832 B audio pool beyond reserve). This
combination reaches 16.014/16.518 ms and 207/206 underruns. It saves
3.279/2.567 ms in peak blocks versus the staged baseline, although Pro's
underrun count is slightly worse than -O2 on its own. None qualifies;
there is no playback release or sound retune. The host keycache-only
wet sweep was bit-identical across all 694 programs in the available
legacy pack; combined opt was separately checked on six presets, each
in wet and delay-only modes. This does not represent the full 906
original presets or compare the firmware to original delay.

The separate delay experiment runs the original left lane through a
legal live edit from 0.25 s to 2 s. It recovers 176,400 historical
input samples bit-exact without new input. Even independently choosing
between just two values per sample requires 22,050 B of retained
information for this witness. This exceeds the extremely optimistic
17,520 B obtained by handing both current 4,252 B effects arenas and
all 9,016 B remaining audio pool to one owner. The bound is scoped to
arbitrary independent input and exact supported retiming, not to
perceptual equivalence or a new storage/RAM layout.

A separate same-timeline offline oracle gives the compact delay perfect
1–4 kHz replacement without time shifts, gain fit, or changed control
inputs. Final-second signed residual remains −13.81/−18.50 dBr (Pad) and
−21.16/−19.38 dBr (Hall); the idealized midband isn't sufficient. A DC–4
kHz oracle does much better on Hall, but still fails Pad's left tail.
Reject a midband-only fix. This is an evidence-only noncausal oracle,
not realtime DSP or an audible-parity acceptance.
