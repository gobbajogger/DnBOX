# Synth One original Tremolo LFO ID59, Init-only

Six app-only files against exact `controls-init-only/source-all-wheel`,
`ALL-WHEEL-POST.sha256`, all entries verified. Zero-fuzz patch, no
firmware DSP changes, preset, hardware, Git, upload, native or packaging.

Pinned `S1DSPKernel.hpp:415` ID59 native OFF/LFO1/LFO2/BOTH domain, default
OFF; `S1DSPKernel+process.mm:172–190` uses the descending LFO, or the mean
of both, in the post-crusher mono amplitude before stereo pan. Compact
uses the identical route algebra and ordering. Real PITCH MOD K3 in the
existing group after ID55 and ID57; no new page or invented range.
No claims about exact original waveform generation, crusher timbre, post-
tremolo effects, or per-preset authoring beyond this verified receiver.

Previously invalid high bits of S1 q7 values [0..2] independently hold
0 absent (exact authored prior word) or 1..4 explicit, typed by existing
high mod marker; reject 5..7 and inconsistent marks. Low seven bits retain
historic bend, damper and crusher. Reads and the genuine crusher writer
preserve their own physical fields. The historical migration predicate
cannot adopt marked new records; externally held wheel has no path to
this wire. No ARP or [21] ownership claim. Existing ID55/57, byte30 group,
peer, and per-pattern/save/note wire lengths untouched.

96,518 shift-sanitized checks: actual SMK K3 and four-state selected
readback, original formula independently observed before/after the genuine
post-crusher multiply on every one of 4,096 frames per state/owner, 4,096
affected mono samples for each active route/owner, whole PCM depth-zero
exact, opposite owner bit-exact. Full actual persisted project recall,
lock apply/edit rejection/unlock, existing own ID55/57, FX, FM/Decay,
level/depth. A real external MIDI pitch message and center readback
leave whole Sound and Tremolo unaffected while physical crusher edit
preserves the typed high bits and compatibility marker. Exact old reader
refuses both new projects and attempted subsequent save (104 checks).
Untouched Init 4,096 stereo frames bit-exact; no host owner growth,
paired original_bass still 0x2b20. No native fit/cadence claim.

Replay: scratch copy exact preimage, apply `--no-backup-if-mismatch --fuzz=0`.
Run `python instrument-host.py SCRATCH_SOURCE` to install the passive math
observation and owner accessor on the scratch source only. Run
`sh run.sh [W [SOURCE [OUT]]]`; old parser `TEST_FLAGS=-DS1_TREM_OLD=1`
needs no instrumentation and consumes actual fixture exports, exact
previous no-editor `-DS1_TREM_RED=1` needs at most the host accessor.
Set `S1_TREM_PCM=FILE` for untouched Init dump. No private PCM here.
