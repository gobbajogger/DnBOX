# A uniform seeking store, no warm migration

Diagnostic only. Five independent causal age windows are fed continuously,
each storing identical signed 6-bit floating values and random-accessible
stereo pairs (three bytes per two stereo sample cells). There is no format
change, migration, seek decoding, memset, or data-dependent loop. The
requested age can change on any sample. Existing Delay receiver ramp and
LFO state can remain outside this store: this probe does not replace them
or imply that this is a qualifying receiver.

| Tier stride (raw samples) | Cells | Selector upper age |
|---:|---:|---:|
| 1 | 48 | 46 samples |
| 5 | 2,224 | 11,110 samples, 251.9 ms |
| 512 | 96 | 48,128 samples, 1.09 s |
| 8,192 | 112 | 901,120 samples, 20.43 s |
| 65,536 | 130 | design covers existing 8,372,224 samples |

Ring 3,915 bytes, 3,988 complete measured target state including overlap,
phases and running sums. Fits in the 4,096 B owner-local arena with 108 B
left. Worst work statically 5 accumulator pairs (10 float adds), up to
five writes, two random 12-bit stereo reads and their interpolation. No
recursive or burst decoder. Layout-only target code 1,004 B text +42 B RO.
Existing frozen app margin +32/+40 B, so no native fit claim. Host
component proves exact quiet bypass and callback partition equality for
fixed authored delay, not production automation.

Identical 250 ms exact reference sweep (see result.json):
- 3 kHz first echo -22.094 -> -4.555 dB, second -22.483 -> -8.206 dB.
- 1 kHz first/second -0.469/-0.450 dB; 6 kHz RMS measures
  -10.363/-13.587 but null near zero dB, so this is not real restored HF.
- Fixed structural failure: immediately over 251.9 ms selection moves
  from stride 5 (8.82 kHz sampling) to 512 (86.13 Hz). At 252 ms the
  fitted design still uses stride 16 (2.76 kHz sampling). The prototype
  therefore regresses real mid/long signal even though no time detent is
  removed. A simple cutoff crossfade cannot replace missing history.
- A 6-bit stereo format supports at most floor(4096/1.5)=2,730
  cells even ignoring state. At 250 ms and stride 5 ~2,205 are needed.
  Retaining a conventional fixed per-octave age density around each old
  cold-selected function's capacity needs roughly half a 1,024-cell
  ring per extra octave. ~525 cells cannot cover all supported octaves
  at that density. This falsifies naive simultaneous fixed windows, not
  every possible future adaptive age promotion architecture.

Do not graft. To get an accepted shared store, implement and prove a
different adaptive timestamped density with bounded migration or prove
a separate much stronger coding ratio. Don't hide missing long-mode
bands with a mode-specific blend or a level trim.

Reproduction: python sj6-uniform-age/measure.py. The frozen/reference
bridge is the same as sj6-fit-delay-audit, with no data uploads.

## Follow-up: aged forwarding, small adjacent ratios

A second isolated source window.c implements 12 tiers, first rates 1/5/10
then repeatedly ×3, up to 196,830 raw frames per cell. Each stage
only receives older cells leaving its previous age stage, eight cells
ahead of retirement, reducing duplication of the 250 ms region. One
chain promotes at most 12 cells per rendered frame; no edit initiates
a seek or refill. Timestamped arbitrary-age read checks a maximum of 12
tiers and reads at most two interpolant pairs, using up to four overlapping
fine cells of adjacent crossfade. No cycle-time pass, active DSP startup,
production ramps, or full-range authored timing claim.

Source-bound PI32v2 state 4,080 B, so all age tiers fit per owner with only
16 arena bytes unused. The isolated code/table cost, however, is 1,232 B
text +108 B RO, not shown recoverable from guarded +32/+40 APP.
boundary.py advances 9,175,040 input frames and checks 185,281 uniformly
sampled and fine-grained queried tap ages through 8,372,224 raw frames.
A .125 binary-exact constant remains exact in the left lane and absent
in the right (no holes). This verifies address/time coverage on a
controlled signal, not arbitrary warm ramp sonic fidelity.

At 250 ms, 3 kHz 1st/2nd echo error -4.403/-8.198 dB, vs fitted
-22.094/-22.483 dB. The adjoining 5→10 transition is gentler than 5→512
in raw sampling step but not a quality pass: 3 kHz correlated amplitude
-12.475 dB at age 11,152 drops to -23.575 dB at 11,162, while 1.41 kHz
alias rises -14.387 to -10.565 dB. Crossfade is smooth on a constant,
not enough for audible timbre. Preventing aliasing requires another filter
or history representation and more memory/flash/time proof. No graft.

Run measure-window.py, boundary.py and sweep.py independently for the
numeric receipts. Stable state probe size is window-layout.txt.

## Earlier spectral rejection at the 5→10 transition

window-aa.c swaps only the promoted second tier's two-cell box for a
center-preserving 6-cell binomial [1,5,10,10,5,1]/32. The older cells
already exist inside the 8-cell promotion guard, so neither time age
nor state bytes changes. No filtering at 250 ms itself; 3 kHz stays
-4.724 dB on correlated level there. At age 11,162 raw frames the 1.41 kHz
fold image drops -10.565→-36.674 dB (26.109 dB rejection), while correlated
3 kHz drops -23.575→-49.666 dB and the 1 kHz tone -1.467→-4.228 dB.
This improves alias rejection, not overall original fidelity. Independent
constant age coverage still 185,281 checks with zero holes. Actual native
object is 1,398 B text / 114 B read-only, with no extra state; this adds
166 B text / 6 B RO and at most six bounded reads each first transition
promotion. Not measured against the firmware tick budget. No graft.
For measurements run the same script names with the -aa suffix.
