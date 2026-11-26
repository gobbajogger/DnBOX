# Synth One 3.75 KiB independent early stereo + late tank: reject
Compared genuine exact Soundpipe RevSC to a quarter-rate (11,025 Hz)
stereo prototype. Early legs are independent float8 input histories
and the four dominant initial same-channel impulse events measured at
authentic source feedback .8; full-clock 4.55-kHz Butterworth front/rear
ladders. The independent late tank uses four short incommensurate
11,025 Hz signed Householder feedback loops, BF16 storage and two
independent stereo injection and projection routes. Late injection
starts after the real approximate first-reflection group. Per-line
gain is the physical residence-time exponent of the authored feedback
parameter; no per-program conditioning or fitted master/audio level.

One objective: two frequency-band interval energies in actual source IRs
at f=.6,.8,.96, .095–1.4s late. A 27-member fixed grid over decay scale,
loop filter and late output gain picks [1.1,.98,.15,1.0]. Its **3.74 dB**
energy RMS is a training metric, not a match. Frozen params were held
out for four independent f values and onsets/impulse directions;
first detectable onset +2..+7 44.1k frames, later signed level and phase
not preserved (e.g. f=.89 held impulses late about -2.0 to -4.7 dB depending on line,
positive null error). The rate passes no original RevSC modulation/jitter
parity claim; envelope/density statistics do not recover its actual
inter-line waveform stereo relationships.

Whole heldouts on latest compressor-cache host donor; untouched exact
note tapes and controls, unchanged delay/filter/voice/compression/widen.
Pad, Hall, Deeper and Organ pre-room L/R/HP/controls are word-exact
at every frame. Whole tails diverge: Organ -9.10/-8.55 dB late level and
isolated room side -17.56 dB; Deeper -15.25/-3.28 dB late level,
5–16k error -61.54/-59.10 dBFS versus exact -93.84/-89.52 dBFS.
Pad signed late relative error -4.32/-16.46 dBr; Hall -2.82/-3.45.
Infinite Tension shows +0.116/+0.871 dB level with +3.10/+3.55 dBr
signed residual. This is an observed stereo-decay and treble-floor
failure, not paperwork. Some absolute spectra small; others clearly
not. No compensation, no project edits, and do not adopt.

3748 B mutable state in isolated host, 2176 early history, 1428 late
history, 144 state. 352,800 samples of independent simultaneous-owner
versus duplicate match all signed words with varying feedback. Four
authored room-bypassed whole candidates word-identical. Compiles in
actual PI32v2 isolated object, .text 1038 B and .rodata 88 B; no native
timing, app linkage or hardware gate. A dual owner would occupy 7496 B
before other room-adjacent state or delay; there is no granted SRAM lease.
The prototype uses four exp updates only every 16 tank samples and when
feedback changed, no cyclic per-sample exponentiation.

Strongest measured non-broken boundary remains the original eight
fractionally modulated delays and full clock **49,380 B history + 440 B
host owner for original kernel** using E6M9: 906 complete tones/tails
verified against unchanged original delay/filter topology, −62.07 dBr
worst whole and −57.43 dBr last non-silent second. Still lossy; do not
assume target fit or exact stereo parity. Same topology+cadence FP8
at 24,690 B/owner fails named musical high bands. No tested lower
successful boundary between these two, no grant of a 49.8kB owner.
A branch-specific multirate codec would be materially different only
if actual independent per-line spectra/energy, valid feedback timing
and branch-modulated interpolation support reduction; using a common
rate factor for all eight lines instead is merely a repeat of the
already failed same-topology common-rate test. Because the feedback
junction shares each line's taps, its high bands can energize nominally
low lines after the first pass. Without a funded memory breakpoint and
measured line-specific benefit, do not rerun another synthetic tank.

To replay source and micro shared objects use the same recovered Soundpipe
headers/base.c as donor (-O2 -ffp-contract=off), then refs.py and fit.py.
The whole graph uses build.sh with ROOM_HBITS=0 (ignored by this model),
EXACT_PITCH=1 EXACT_CRUSH_FLOAT=1 EXACT_ENV=1 and build-releasephase donor.
Compile baseline donor exact; compare.py / dry-compare.py with pinned
existing private capture/command/morph paths. No audio or compiled
binaries/toolchain included.

The complete upstream RevSC source is omitted from Git. The exact checked Library reproducer and manifest have archive SHA256 62654643a9ee1935c7e7b3b7cfe49a23965ac2a5f7e5d0bf64271cb9b09c7039.
