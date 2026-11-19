# Frozen D active-filter harness — source prepared only

This is a separate diagnostic wrapper around unchanged frozen D. No host
compile, static replay, active capture or metric execution has occurred in
this directory. Root review and an execution release are still required.

The sole active contract change from D's physical static host is filter ADSR
`.2/.3/.4/.5` and filter depth `1.2`. Velocity is 100, cutoff 2,000 Hz and
resonance binary32 0.9. Common amp ADSR remains `.05/.005/.8/.05`. Notes 36,
60 and 84 retain their supplied frequency words, fresh per-note state, phase
zero, 176,448 silent warmup frames, 88,192 retained frames and note-off at
frame 66,112, at 44,100 Hz in 64-frame blocks. The exact numeric source is
`inputs/case_plan.json`; the scoped extraction is `inputs/PROTOCOL.json`.

`src/active_host.c` uses the existing D API and copies its tuning/event/render
loop verbatim. Its only two modes are the fixed static baseline and this fixed
active case. The D core and vendor files remain external, frozen inputs; no
filter algorithm, source coefficient, gain or tail policy changes here.

## Pinned numeric inputs

The completed D180 selected static PCM is
`/workspace/scratch/e803955aec60/s1_dry_voice_d/results/audio180/100_2000_.9.candidate-mono.f32le`,
SHA-256 `7aded201280455c1ddaee86abab4d82b6d791953e81d0d9cb40e17946cbfa380`.
Its completed summary is pinned as
`9f4f5d42595c997c01af70b700c934403d86febacb2bf893745fc4934298f352`.

The original active hash was unavailable when source preparation was assigned.
It arrived before this freeze. The sole new original active capture is now
pinned as `3f7303aa1383c678e4e251c36506a141a0ece419e38d4411e2c0a0447bf0bb74`,
1,058,304 bytes. Its allowed numeric manifest is copied unchanged to
`inputs/reference_capture_manifest.json`, SHA-256
`ea1a572e0d0b682a938fc83bdb7f7dd535bb645efd4b749576d039af64710044`.
It reports the matching active ADSR, depth, note/tuning/event contract and zero
nonfinite outputs. This is **new original reference PCM**, not recovery of the
missing historical active pair. No original source, headers, objects, capture
implementation or referenced assets were opened.

## One proposed execution sequence

After root releases execution, `check_active.py` will:

1. Reject output symlinks and symlink parents before resolving or creating the
   provided path. Verify all source, metric, compiler, D, D180-summary and
   numeric-input hashes. Require a new output directory and retain a pre-build
   declaration.
2. Capture verified D, harness and metric bytes once and stage those snapshots
   under the new output's `build-inputs/`. Run one bounded host compiler command
   using only the staged source paths. Freeze its binary hash before rendering.
3. Render the static mode once. Require byte-for-byte identity with D's pinned
   selected static PCM. A difference terminates the task before active capture.
4. Render the active mode once, verify its receipt and finite PCM, pair original
   and candidate streams without changes, and execute the staged, unchanged
   metric exactly once with explicit gain `1`. Errors must be finite and
   nonnegative before the unchanged thresholds are applied.
5. Preserve commands, output streams, hashes and the terminal PASS/FAIL. A failed
   guard, compile, baseline, capture or comparison triggers no adjustment or
   automatic repeat.

The public runner uses explicit runtime checks, not Python assertions. Its
guards therefore remain active with optimization enabled. Output directories,
PCM, receipts and declarations use exclusive creation. The unchanged metric's
output path is checked absent before its sole execution.

The metric SHA-256 is
`bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1`.
The gate stays RMS error at most 1 dB, spectral error at most 3 dB and zero
nonfinite samples, with gain 1, no normalization and no alignment. Static
identity is a separate exact prerequisite; it is not a fitted gain check.

## Exact command after root review and release

```sh
python /workspace/scratch/e803955aec60/s1_d_active_followup/check_active.py \
  --d-root /workspace/scratch/e803955aec60/s1_dry_voice_d \
  --d-summary /workspace/scratch/e803955aec60/s1_dry_voice_d/results/audio180/SUMMARY.json \
  --new-original /workspace/scratch/e803955aec60/reference_filter_inputs/full_dry_preparation/active_capture_proposal/outputs/filter-modulation_1.2_2000_r0.9.original-mono.f32le \
  --new-original-sha256 3f7303aa1383c678e4e251c36506a141a0ece419e38d4411e2c0a0447bf0bb74 \
  --output /workspace/scratch/e803955aec60/s1_d_active_followup/results/active-one
```

This command has not run. `DECLARATION.json` pins the source-only preparation,
compiler and proposed commands. `SOURCE-REVIEW.json` records text/AST inspection
only: the wrapper's sample loop matches frozen D, Python syntax parses, and no
assertion statement supplies a runtime guard. There is no build/test result.

For a relocated source/fixture bundle, supply `--d-root` for `variants/d`,
`--d-summary` for `evidence/d/audio180-SUMMARY.json`, and `--d-selected` for the
saved selected PCM. Both relocated numeric inputs retain their exact frozen
hash checks. Reference PCM may also be relocated using `--new-original`; its
required hash remains fixed. Outputs must remain a new direct child of this
harness's `results/` directory.

The first, unexecuted source-only freeze is preserved under `pre-root-review/`.
The current declaration supersedes its external-source build proposal with
verified snapshot staging, adds pre-resolution symlink rejection and summary
relocation, and requires nonnegative metric errors. The wrapper and DSP,
controls, timing, metric arithmetic and tolerance contract are unchanged.

This one three-note active diagnostic does not qualify the 72 dynamic cases,
arbitrary modulation, 694 presets, all controls/FX, native cost, target fit,
callback timing or production activation. No new D variant is introduced.
