# Fitted SuperJ6 Delay versus fifth-rate 3-bit stream

Identical 250 ms / 120 BPM / feedback .99 × 64/127 / wet .9. The original
arm is the prior Build9-verified full-rate stereo C kernel, not fresh AU
execution. Single-channel 2,048-frame Hann burst, exact authored wet
minus bypass, no aligned/gain-fitted scoring. No effect was removed.

| Hz | Fitted echo 1/2 RMS error | 3-bit echo 1/2 RMS error |
|---:|---:|---:|
| 220 | -0.275 / -0.554 dB | -0.011 / +0.002 dB |
| 1000 | -5.413 / -12.915 dB | +0.113 / -0.474 dB |
| 3000 | -22.094 / -22.483 dB | -0.498 / -2.742 dB |
| 6000 | -22.902 / -24.371 dB | -59.952 / -53.630 dB |

All four cases show bit-exact bypass and zero inactive-channel signal.
6000 Hz fundamental is already missing in fitted output (-65.985 dB first
echo); 3-bit gives -105.679 dB and creates an image at 2820 Hz (-62.246 dB
against the original 6000 Hz fundamental). The 3 kHz waveform residual
still measures -9.642/-8.114 dB relative, not a null pass.

## Can both formats share existing RAM while warm?

Exact target probes: 4,096 B existing owner-local ring + 72 B native
receiver; 4,092 B whole streaming state, including 3,948 B coded ring.
The latter fits in the former as a cold replacement, not as a second
live history. Isolated target code/table size already totals 1,982 B
(1,450 text + 532 read-only, excluding 74 B filter coefficients and
control integration). Both diagnostic linked targets have +32/+40 B
XIP; no evidence there is enough for a second decoder path.

Fixed-stream admission is 77–22,310 samples (about 1.75–506 ms).
The exact source retains shortest 1 sample, free maximum 1.5 s, and the
18-detent sync range up to 16 s at 120 BPM, higher at slower
tempos, with independent AUX LFO time window provisioning. Existing
processing ramps time, feedback and wet over 10,000 samples in 8-sample
control intervals. The standalone stream does not carry these semantics.

The two-format warm crossing needs one of:
1. **Crossfade separate live read heads.** At 250 ms old stride 16 needs
   at least ceil(11025/16)=690 × 4 = 2,760 B of readable old history.
   New stride 5 needs 2,205 cells = 46 × 42 = 1,932 B checkpoint blocks
   plus 144 B working state. Even counting only the active duration,
   total lower bound **4,836 B**, before guard reads or coefficient
   state. Full retained histories 4,096+4,092=8,188 B.
2. **Synchronous in-place transcode**, with no fade. Forward crossing must
   still touch 690 old and at least 2,205 new tap cells. Backward crossing
   may decode a full packed history (up to 4,462 valid stereo cells).
   Decoding is predictive; old ADC input history is already lost,
   conversion changes samples. No transform satisfying read-before-write
   and immediate arbitrary live retime has been implemented or measured.
   A sample-time switch cannot rely on this unbounded callback spike.
   Existing smaller retime test had a measured 55 stereo frame seek in
   one rendered frame even without switching history representation.
3. **Deferred incremental transform.** Requires reserving old and new
   critically read intervals or an explicitly multiphase map, while
   future live arcs can use any currently retained window. No admission
   of delayed automation, erased history, or a temporary longer-mode
   rejection is permitted here. No workable exact schedule exists in
   the current source.

The memory bound is for the straightforward independent heads design,
not proof that every possible future mixed-layout or migration
algorithm is impossible. Given both the measured XIP budget and the
missing bounded warm semantics, no variant was grafted or declared fit.
An architecture with stable per-block format tags and separately bounded
decode work would need its own source, XIP and audio timing proof.

Use the frozen compact bridge as in fit-s1-ab and run
python sj6-fit-delay-audit/repeats.py. native-layout.c / native-layout.txt
are minimal ABI receipts. Keep private inputs, emitted .o and PCM out of
any handoff. No production/device/emulator work.
