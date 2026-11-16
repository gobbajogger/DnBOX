# Sound v2 pre-FX component evidence

Base: e742be7bd0d01323c899de6e156f40b85ee451db.
Reused adapter/test donor: bd60550833daa862ed5aca48e7af6b3d8d361de2 (PR #952).
Pinned original source dependency: 56079bbdcb753c37d65aedd17ce49ba71c261797.

The previous runner verified retained source hashes but reused preexisting object
files, so source hash verification did not establish what had been compiled. The
new runner rebuilds every compile command into a separate output directory and
rejects any link object not produced by that plan. It preserves source inputs.

Regression: `python3 tests/host/test_synthone_rebuild.py` failed on the donor
runner because the deliberately invalid dependency source was never compiled.
It passes on the new runner. Independent review additionally caught unbound
compile paths and an optional adapter substitution. Two new negative regressions
failed on that version and pass after binding every actual compiled source and
include directory to a complete `CLOSURE-SHA256.json` snapshot, rejecting unlisted
closure files, and requiring exactly one adapter substitution. All three
regressions pass. The 89-file retained closure snapshot is an input-integrity
record, not independent upstream-source provenance.

Actual run: `python3 tests/host/run_synthone_renderer.py --source-build <trusted-source-build> --output <fresh-output>`
compiled 28 objects from retained closure sources and ran successfully:

```json
{"scope":"new-adapter-vs-actual-source-pre-FX","scenes":12,"frames":576000,"different_channel_words":0,"nonzero_left_frames":529280,"peak":0.628623188,"heap_calls_during_render":0}
```

Known warnings: upstream Soundpipe ftbl signedness comparisons and unused
variables; preserved original compile policy permits these warnings.

This does not establish all factory presets, post-note FX, independent original
application rendering, device xruns, target binary reproducibility, or Elite/Pro
all-engine fit. S1/S2/S6 remain FAIL, and Local gate has not run. The retained
commands and source hash list are trusted build inputs, not a sandbox for arbitrary
third-party commands. No raw audio or toolchain binaries are included in Git.
