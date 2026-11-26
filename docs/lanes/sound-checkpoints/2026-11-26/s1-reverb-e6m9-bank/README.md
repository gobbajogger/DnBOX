# Synth One wet-history host witness, 906 matched presets
Scope: latest recovered exact 44.1k host voice, controls, tuning table,
same six-second note events in both arms, latest compressor-cache 906
donor. The unmodified candidate PCM SHA matches each pinned latest bank
receipt before interpreting any variant. No parameter edits, per-preset
choice, gain, resampling, normalized time offset, hardware, or fit claim.

History per owner: stock 44.1 kHz four Soundpipe Float32 vdelay rings x10 s
7,056,016 B; two long (2t) and two short (t), with linked right crossfeed
but all four original exact recursive readbacks. With t=.2162 s the
packed-signalled Float32 history lower bound **228,832 B**, not allocation.
Eight RevSC independently modulated delay lines need 24,690 samples,
98,760 B (separate 440B host RevSC object includes 8 interpolator/cursors,
filter states and jitter states). One right-channel Widen 50 ms/2,205
frames needs 8,820 B. This gives 336,412 B/owner or **672,824 B/2 owners**
for that tap-specific lower bound, versus 14,327,192 B actual 2-owner
allocated history. 48 kHz independent module measurements (not full
native/whole graph) are 7,680,016 / 107,440 / projected 9,600, or
15,594,112 B actual cadence-based comparison. Repeated live independent
owners and a solitary duplicate were bit exact across 88,200 (44.1k)
and 96,000 (48k) signed frames per representation. There is no safe
flash relocation for these histories and no shared simultaneous state.

At constant original cadence and with all junction, cubic, feedback and
random phase operations unchanged, FP8 E5M2 was audibly-risky (Organ
tail signed -21dBr, Pad treble -13/-19dBr) despite 75% room RAM saved.
Plain BF16 halves room memory and is much better; final 16-bit E6M9
with explicit round-to-nearest even uses the same 49,380B/owner and has
4x BF16 significand precision. All 906 whole cue comparisons completed;
230 rows byte-identical. Worst whole signed residual -62.073 dBr (459/L),
worst non-silent final second -57.430 dBr (838/R), worst whole level
delta +.000112 dB (378/R), worst last-second non-silent level -.001617 dB
(756/L). Both level signs present, not hidden fixed gain. For named
five audited rows the complete pre-room stereo, controls and RevSC input
records are word-exact. Pad rows62/369 final tail -68.8/-82.3 dBr,
5–16 kHz signed -55.6/-60.7 dBr. Organ row113 tail -63.8/-64.6 dBr,
5–16 kHz +1.02/+0.91 dB with error actually -107.13/-105.66 dBFS,
stereo mid/side levels within .00019 dB. On Deeper Space Pad row366
5–16k +12.87/+9.50 dB is a weak-source denominator, reference
-93.84/-89.52 dBFS and measured absolute codec error -81.20/-80.49 dBFS.
Its whole stereo tail null is -64.65/-66.21 dBr. No perceptual assertion.
The band dBFS uses one-sided interior FFT power divided by N*sum(window^2).

Combining the independent prior exact shorter two short rings (keeps
low-level legal bench t<=5, new buffers 5s +8 frames, virtual 10s phase)
with E6M9 leaves **10,700,544 B** of actual 44.1k two-owner history:
3,626,648 B saved /25.31%; all 43 combined holdouts byte-identical to
room-codec-only. 48k independent allocations sum 11,646,800 B including
projected Widen, not full graph. This is not close to current hot SRAM;
two E6M9 reverb histories themselves are 98,760 B. At the .2162s bound,
shortening the overallocated FFT-free reference histories helps no actual
effective delay, and E6M9 gives 574,064 B vs original 672,824 B. No counts
pretend this alone solves target fit.

Rebuild from audited workspace with room donor under build-releasephase;
first run generate.py, then build.sh with ROOM_HBITS=8/16/169 and
EXACT_PITCH=EXACT_CRUSH_FLOAT=EXACT_ENV=1. See inputs.sha256. Scripts
reference existing donor and exact private cue/input files by path, not
contained recordings, toolchain or build outputs. combined.sh uses the
reproduced earlier shortened exact-delay.c/h; callers control jobs.

The Git checkpoint omits the complete upstream RevSC source and derivative generated from it. The exact received reproducer, upstream identity and checksums are in the companion Library archive, SHA256 781b6d6aa68b3235703594b2cc8460bcd96c8a6f8c5d2816e1a1f19168a8f6af. This checkout preserves the first-party scripts and measurement records.
