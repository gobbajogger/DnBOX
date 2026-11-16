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

## Exact input and invocation records

SHA-256 checksums (local trusted retained closure; no raw assets published):

- `source-build/CLOSURE-SHA256.json`: `bb32c1e7d5518586ba33dfb68ef700c5db3fded87183e2298504e90597ab7125`
- `source-build/COMPILED-SOURCES.json`: `f955cb8ab9f57e404ae8fba3a1668c5d1bc25a2d58ce227515cda41e06fb109d`
- `source-build/commands.json`: `2e272154660d48449e278c3205fff0d78200b23a7c2649685bc08c57f3f937c5`
- `fresh-output/commands.json`: `75f73576f15e6352ee39c97af7fcb0a76ccf63a73a24a6cb6be4fb5bd2e043bd`
- `source-build/public-morphs.f32`: `0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5`
- `source-build/compact.bin`: `a5cf8182b8bc9ca0a1b034a6b63f55d2cdc1087df38d6b1433da4fb56e5eccd5`

Independent test-and-merge review at e3e156c1323840d9db516e5aad6940b2cff71cb5: source-path binding and harness guards inspected; all three regressions independently rerun PASS. Bounded component-integrity approval only, not rule 9 Local approval.

## Caller-owned C ABI checkpoint

`firmware/src/dsp/synthone_c.h/.cpp` adds an explicit storage/span/alignment API
for cold init/retire, resolved physical parameters, public note events, reset, and
stereo render. It is pre-FX only; no existing app bank is enabled or reinterpreted.
Config, compact pack, sine, bands, parameters and tuning cannot alias owner storage.
Stale/forged handles are outside the documented lifetime contract.

Expanded actual-render test first failed to link the absent API, then passed with
the implementation:12 scenes/576000 frames; C ABI and C++ adapter bit-identical to
source oracle. Boundary cases include null/short/misaligned owner, config and
parameter overlap, invalid parameter index/NaN, invalid note/velocity, reset and
retire. Three build-integrity regressions still pass. Native wrapper compilation
with exact pinned C++11/S1_NO_DOUBLE recipe passes; undefined-symbol scan finds no
allocation or software-double helper. Independent source-only review found no
blocking issue; independent test replay pending.

Final fresh command manifest SHA256: `c653fcd2bd45410691ae49a27150125cb6a1614002d7ce01a36d027e0a51693d`.
Host summary unchanged:576000 frames,0 differing channel words,529280 nonzero left
frames,peak0.628623188,0 heap calls during render. This adds a caller seam, not a
firmware link, timing, factory or all-engine memory PASS.
