# Original four source fan-in and one proven pitch-coordinate defect

Eight installed official newer Mac AU renders: same Brainwave cue and
untouched active control vector except OSC1/OSC2/Sub mutes. Both native and
compact passes use F32 44.1k chronological frames, real documented samples,
fixed same callback boundaries and fresh state. Actual native and compact
same absolute scope except deliberately muted legs. Noise live zero on
every original arm; not measured in isolation or claimed as solved.

Signed all vs sum of signed singles: native held −65.39 dBr, compact
−132.55 dBr. Thus the observed relative cancellation is not nonlinear
cross-source mixing. Actual measured held Sub-only fundamental is 33 Hz
native, 66 Hz compact before change. Both Main frequencies are in right
octaves. Original active uppercase RANGE=0 source sends note 48→36 before
Sub source selection; the sampler's harmonic is an octave below that key.
Stock used pre-Range MIDI note for both pitch_increment and SFZ-zone
selection, so it synthesized a whole octave too high with wrong zone.

Narrow source-only guard forwarding recognized typed Range to note and
zone (no synth phase, onset, source wave, gains, mix, filter or FX changed).
No native operator writes. Input out-of-range note coordinate does not
establish original clamp behavior and remains legacy (no fabricated policy).
In actual controlled lower Range arm, compact after holds 33 Hz; acoustic
sub-only held still mismatches spectral/phase details and worsens ~.34 dBr
signed. Both main-only arms remain byte-identical, center-Range synthetic
byte-identical and upper synthetic 66→131 Hz (not native audio validation).
Unsupported routings/positive values unchanged except stated typed range law.
Armed routes remain preserved. No app still installation or hardware.

Dry fan-in all-sources level error originally −37.64 held / −36.13 chord /
−35.33 release; after −33.12 / −31.70 / −30.90 dB. Pending level instead
worsens 1.33 dB even as signed residual improves. Simple signed sum remains
additive. Whole matrix (different FX arms and authored source controls, not
same source-only window) Brainwave +4.09/+4.16 dB authored/dry, Raw
+7.95/+7.99 dB, Solidify with Sub off exact. Dry Raw signed improves .095 dBr
and spectral RMS .56 dB. No change to Master; no overall/per-preset gain.
Not a shipping fix: sampled Sub versus compact surrogate, live independent
cursor/envelope/filter handoff and large amplitude mismatch remain.

Reproduce:
  python3 sub-range.py DONOR/firmware/src/dsp/dnbx_core.c COPIED.c
  SJ6_DNBX_CORE_OVERRIDE=/abs/COPIED.c ./build.sh DONOR /abs/renderer
  python3 fanin.py AUTH_AUV3_FANIN.tgz RUN_FOLDER /abs/renderer > RESULT.json
  python3 range-score.py AUTH_AUV3_FANIN.tgz BASE_RUN RUN_FOLDER
  # separate authored/dry tests use prior export.py and score.py
Pins in .pin and .sha, no actual PCM or original assets in package.
