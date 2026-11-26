# Exact Synth One paired cubic geometry
The two long and the two short source delay lines share physical and
logical cursor/size and execute once each at each sample. Exactly two
pairwise tap ages are needed. This implementation computes two cubic
addresses/weight sets, retaining all four independent audio buffers
and feedback recurrence states; line3's write still uses the *same
sample's* line1 actual output, and right fill add is unchanged. It
keeps reset semantics (only line0's cursor is read by this adapter,
but actual Soundpipe reset still clears all four and leaves previous
feedback output just as before). No temporal interpolation changes,
feedback retuning, room/delay bypass, or reduction in range.

On a finite Float32 time and exactly representable integer sample
rate, a binary64 multiply of two binary32 inputs has no intermediate
rounding before narrowing, so the single original narrowing equals
the Float32 multiply in the final variant. Fractional semantic
details such as the decimal 0.1666666667 weight stay double. The
native runner proves compensated sample chronology with 12 exact
signed PCM FNVs; complete host lane uses direct byte equality.

All three variants independently passed 906/906 wet PCM and note
tape hashes. Receipts are byte-identical. Active held editor proof with no external
ARP tape: 4 cases, 2 genuine ARP states, 24 live settled-target writes
using actual panel frontend time/sync/BPM/feedback/mix, direct full
weighted outputs bit exact while playback continues (live-replay.log).
Isolated signed host proofs at 44.1/48 kHz, 2,200,960 stereo samples
per pass/arm, active LP and fades vs tap-only, alternate measurement
order, live frequency-dependent feedback/reset and logical wraps
including 0 and 5 s low-level edge diagnostics. This adapter does not
change which controls the public frontend exposes; 5 is low-level only.

Native strict isolated diagnostic holds four 1,024-sample rings in
private virtual RAM instead of trying to fit stock 10-second lines.
TIMER4 sampled per 512 stereo frames at 24MHz under same emu gate.
Old 5,928–6,244 us; inline 2,907–3,092 us (+452 B linked .text);
shared-body 3,477–3,652 us (+96 B);
final F32 paired 2,325.58–2,377.92 us (-8 B linked .text; 2,292 to 2,284).
~61–62% improvement for this exact four-line **tap diagnostic**, not
current compact DSP callback or hardware. Signed outputs unchanged,
no persistent buffer/reverb RAM reclaimed. New stack temporaries for
two geometries, no new owner fields. A one-temp branch reduced caller
stack but its non-F32 variant cost more code with no timing advantage.
Native .text differences are diagnostic linker GC; other linked users
of the old generic symbol can prevent reclaiming all of that text.
Eliminating double helpers reduces cycle cost, not a guarantee for
an app that already used different FPU flags or kernel call ordering.
No writes/erases in virtual NOR, no emulator shell changes, no hardware
or actual shipped firmware. No claim full-rate histories fit SRAM.

Standalone host and native replay compile the prior recovered Soundpipe
source, modules/vdelay.c, base.c and Moogladder as called by the exact
donor. Whole graph script wants donor build-releasephase and scratch
sources render.c/s1c.c/tuning-load.h from unified-releasephase, then
ROOM_HBITS=0 EXACT_PITCH=1 EXACT_CRUSH_FLOAT=1 EXACT_ENV=1. Run same-bank
with no args; inputs.sha256 pins exact reference. Package no audio,
FWSC, executables, toolchain or private app binaries.

Note: the paired adapter owns all four cursors for its lifetime; only
line0 is published internally. Do not live-switch an existing owner back
to an uncompensated generic cursor array; the copied source adapter
contains no such path.
F32 host median tap-only process time: 28.7% faster @44.1k, 25.6% @48k; active-LP +1.5/+4.7% on adjacent runs (no loaded meaning).
