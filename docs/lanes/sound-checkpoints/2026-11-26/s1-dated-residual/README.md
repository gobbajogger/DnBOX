# Dated Synth One residual storage: no admission
Five authenticated 44.1 kHz exact-source delay captures, 264,728 frame
records each, including 128 startup frames. Existing four Pad62,
Hall151, Mushroom191 and Deeper366 feedback traces reused; new Infinite
Tension FX459 capture used same proven original source recorder and
factory input, is >1.817 s/.8997 feedback. Exact native Soundpipe vdelay
source rerendered from traced inputs/controls, preserving long L/R,
separate short fill and exact short crossfeed from reconstructed long R.
Every one of all four captured tap-equivalent fields matches bit for bit
at all frames: long L, long R, short fill, signed summed short right.
Old vs current checksum is attached, no protected PCM included.

Observe each independent aged ring in true physical sample coordinates.
Zero-phase host-only split at 2.756/5.512 kHz; the latter is an intentionally
favorable test that forgives essential bands below it. For each snapshot,
keep the K largest signed-RMS-energy blocks among all live dated blocks,
knowing their entire waveforms and with a perfect exact-low assumption.
This is more generous than real causal admission; best for history energy
for this fixed segmentation, not necessarily best for one particular tap.
Original nonlinear source order is not changed; tests retain its exact
feedback state to measure only signed omission/nibble error through true
cubic read addresses (coefficient eval double) and right crossfeed sum.
This is a storage feasibility bound/witness, NOT an estimate of solved
closed-loop playback, clicks or target cycles. No source gain normalization.

Per block: 64 gated full-rate old samples, 32 bytes signed nibble with
peak-derived FP16 scale (2 bytes) and 16-bit key (fits line2 + absolute
epoch modulo 2^14 blocks, > any valid oldest age). 36 bytes before lookup
directory/map, source journaling and any other effect state; low is a
favorable indexed 4 bits per downsampled sample, no metadata and exact audio
when scoring high. 128 bytes reserved minimum state. Smaller caps are
illustrative optimistic ceilings; 4534 B is the entire hypothetical
two-owner wet replacement pool equally shared, not an actual delay lease;
22000 and 25792 B per owner are only generous diagnostics.

At Pad t=.2162, D8 low payload 3580 B, 826 B remaining after state at 4534:
22 of ~898 date blocks, about 2.45%. Near 5s the 2.756k+ history needs
663/862/893 of 898 (74%/96%/99.4%) even for 90%/99%/99.9% energy.
Best 22 keep about 10.6%; signed omission/nibble error about -25.5/-21.5 dBFS
on next-two-channel Cubic wet-stem reads (no master/reverb/gain). Even
favorable 25,792B D8 only keeps 87.2% of dated-history high energy at that
point, raw wet tap error -32.8/-33.0 dBFS. Retaining all 898 high blocks
would spend 32,328B before the low lane and already leaves 4-bit
quantization noise in the isolated history.

Hall t=.6665: low alone 11026B at D8, impossible at optimistic 4534.
At 25,792B, 406/2760 dated blocks at 5s (2.756 kHz high); 28.5% energy
kept vs 2045/2640/2741 blocks needed for 90/99/99.9%.
Infinite Tension requires 30,060B low alone at D8. D16 at 25,792 has
only room for 295/7520 dated records at 5s; 60.7% of high energy, nontrivial
wet tap error (-46.6dBFS in right) remains. Deeper/Mushroom 5.5k+ can be
tiny enough to omit at some phases, but their 2.756k+ stress snapshots
still approach -71dBFS raw error with the small budget and do not repair
fixed global dropping of older content across programs. -80dBFS is an
explicit forensic omission floor in this witness, not an adopted product
psychoacoustic release tolerance. dBFS here uses a unit-peak raw float
scale and reports unnormalized internal stem, not listener gain.

Decision: reject this fixed-age pool, with no broader impossibility claim.
Keep exact host reference and present compact control ranges. More clever
coding would require evidence of coherent structured compression across
dated distributed energy, finite reads and real owner headroom; not a
threshold retune or room/delay sharing assertion. No large benchmark,
native timing, firmware modification, device write or release.

Run score.py against existing verified write/state traces, generate .state
with writes.c against exact Soundpipe vdelay/base and standard host compiler
-ffp-contract=off includes. Existing trace paths and executable SHA are
listed in inputs.sha256. The 459 input was made with S1_FEEDBACK_TAP and
the prior .write-build witness, same factory morphs/commands, wet mode.
