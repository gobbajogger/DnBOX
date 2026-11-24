# Original Synth One Mono and Glide on Init-only

Apply the seven-file patch to exact `controls-init-only/source-s1-detune`
using the checked preimages. OUTPUT K3 (POLY/MONO) and K4 (glide in ms);
existing 15-page count; original IDs 20 and 21, not Mono Legato ID120.
No banks or factory data, physical changes, Git or native build.

Original `DSP/S1Parameter.h` and `DSP/Kernel/S1DSPKernel.hpp:376–377`:
mono off/on and glide 0–0.2 seconds/default zero. Original
`S1DSPKernel+process.mm:58–68`: rates of the filter applied to mono frequency
are per-sample; glide is exponential half-time and mode changes clear notes.
Compact already uses the same half-time law per sample, with stock smoothing
and an independent monophonic path. The new 5 ms manual detents are a DnBOX
design, not original continuous UI parity; default/untouched exact words kept.

The old upper seven bits of Synth One base Sound[6] carry independent
absent/off/on mode and absent/0–200ms glide, radix 42. Low bit remains
Amp→Filter link, normal link edits preserve mode/glide. Dedicated high
bit of Sound[20] is required iff mode/glide is present. This is not
optional: [6] alone can appear to be a historical slot-2 payload that would
otherwise migrate silently. [20] is old-invalid and blocks historical
adoption. Its occupied lower seven bits remain AMP release and both
write directions preserve independent values. No wire lengths or shared
row claims change. Reject every invalid digit and missing/extra marker;
no factory data provider or old migration changes.

An actual sequenced-note-lock probe found the initial implementation
silenced the locked first note: after the lock changed the mode and
(s1c_start) started a new note, the stock per-sample reset cleared it.
The patch factors the exact existing reset; if a mode change is pending,
start resets old voice state before admitting the new note. The render
fallback remains for standalone mode changes, with no increased cadence.
Regression exercises the real note onset and unlocked return.

Shift-sanitized Init-only test: 9,552 assertions with real OUTPUT K3/K4,
independent legato/detune/link/AMP release, half-time C4→C5, Poly+glide
same PCM, Mono+glide selected output changes/true peer bit-exact, live
note handoff, cold reload, locked and unlocked notes gated and applied.
Both owners at sample 8704 observed 391.281 Hz vs ideal 391.240 Hz;
finite precision and stock parameter/phase cadence are not expanded into
portable original A/B claims. Four actual older-reader cases, including
glide-only near the historical-migration ambiguity, reject load and save
without writes (208 checks). Untouched Init stereo 4,096 frames byte-exact,
host complete pair owner storage still 0x2b20. No native fit/timing claim.

Replay as previous route packets: copy exact original once and apply
patch. Append the host-only owner accessor from the ID30 handoff to the
scratch engine.c. Run `sh run.sh [W [SOURCE [OUT]]]` with the ledger and
`checkout` adjusted to your scratch folder if needed. Positive run exports
its actual projects. `TEST_FLAGS=-DS1_GLIDE_OLD=1` on exact prior source
exercises the decoder against them. Prior missing endpoint is
`TEST_FLAGS=-DS1_GLIDE_RED=1`; set `S1_GLIDE_PCM` to capture untouched Init
PCM from either side. `lock-red.log` preserves the observed first-onset
failure from the otherwise-working candidate before reset ordering fix.
