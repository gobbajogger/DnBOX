# Firmware structural defect first slice

Local source-only tests for real firmware host renders. This is not full UI,
all-24, native firmware, device, flash-fit, timing or delivery acceptance.

## Scope

- Check1: typed engine references, unique per visible control. Repeated clipped
  passes at the same intended origin are coalesced, not called duplicates.
- Check2: repeated glyph type plus same typed reference; separately, actual Pitch
  carrier-ratio controls must not route through the Tune tuning-fork renderer.
  Distinct Tune/Pitch references are never falsified into duplicate parameters.
- Check3: actual rendered panel content bounding-box area >=70%; exact largest
  empty integer-pixel rectangle <=12% of the full240x240 screen. Panel fills,
  borders, padding and touch areas are not content. Ink includes actual text,
  glyphs, body pictograms and step marks. No semantic exemptions or added padding.
- Check4: actual painted captions, >=2 characters, reject P.DPTH and P.DEPTH. The sole
  single-character whitelist is conventional filter Q. Numeric values and
  parameter names inferred without painted text are not captions.
- Supplemental D01: actual glyph/value raster-bounds collisions. Not a claim
  that full clipping/contrast/typography check19 is implemented.

All22 drum voices and every real page are constructed using the existing host
`setup`, `build_pages`, `ui_frame`, `draw_main` and `draw_knobs` paths.45 drum
voice/page screens result in the pinned fixtures. Other screens/domains and
human scoring remain NOT_RUN. The host's existing hardware/clip substitutes
remain explicit. Every frozen input is hash-manifested, copied with symlinks
preserved, checked before hooks, and left unchanged. Only gfx text tracing,
drum drawing tracing and a test capture source change inside the isolated copy.
The changed-path allowlist is enforced.

Actual clipped raster deltas are coalesced across bands. Caption presence is
also independently checked against **opaque pixels in the actual native font**
at its intended global coordinates in the final framebuffer. This avoids
mistaking a zero incremental paint in one band for absent final text. A caption
requires >=90% opaque-mask pixel matches; match count and denominator are kept.
This is an automation threshold, not a human aesthetic score.

## Run

```sh
python3 -m unittest discover -s tests/ui_defects/first_slice -p 'test_*.py' -v
python3 tests/ui_defects/first_slice/run.py \
  --old /path/to/ui-formatter-freeze-2 \
  --current /path/to/ui-source-freeze-3 \
  --output /new/private/output-directory
```

Output directories must be new. All source closures, checked hook changes,
first host exits/logs, complete traces, actual PNGs and per-screen reports are
retained. `run.json` describes the host compile/render invocation; renderer
exit0 does **not** mean detectors or UI passed. `report.json.detectorExit` is
separate. The combined runner returns nonzero if either source has a detected
defect. Nonimplemented checks are NOT_RUN, never PASS. No Git/network/Actions
or device operations occur.

Run an actual negative case on each source, with unchanged detector logic:

```sh
python3 tests/ui_defects/first_slice/regression.py \
  --render-dir /output/old --case D01-inactive-multi-control-collision \
  --output /new/old-case.json
```

Exit1 means defect caught; exit0 means that isolated case is clear. Neither
means the complete candidate passed. Cases include real old D01 collisions,
a sparse808panel and unpainted compact captions. Duplicate typed-ref unit cases
are explicitly synthetic calibration because no authentic past duplicate-ref
source has been recovered for this slice.

## Historical semantic fixture

Freeze2 already contains the Pitch carrier-wave correction. It is not the
historical duplicate-fork source. `historical_glyph.py` verifies the authentic
`f254bf2c890623d6d543ed3b874ea225117f1290` blob against the saved GitHub API
receipt and composes its unchanged `dv_glyph` function body into the frozen
current host source. The remaining current source and signature are retained.
`COMPOSITION.json` records exact hashes and limits. This is a source-backed
historical-function fixture, **not** recovery or qualification of the complete
historical firmware. It runs the same actual renderer and detectors.

No raw private capture, SDK, toolchain, generated host binary or instrumented
source copy belongs in a public commit. Retain private evidence separately.
No source qualification, integration, push or publication is performed here.

## Fail-closed producer revision

`expected-screens.json` is an explicit static45-state inventory, not inferred
from candidate output. Empty, missing, duplicate, extra or misidentified states,
wrong dimensions, missing binary raster masks, missing positive glyph/panel
structure or missing final native-font audit reject parsing and saved-trace
evaluation. An empty structure is NOT_RUN, never PASS. Supplemental actual-font
text bounding-box intersections now include the WIP TUTUNE section/control
caption duplication. These are measured text bounds; full ink-level typography
and unrelated FX/Home/Drone/Remote defects still require their own fixtures.

Regression receipts use exclusive file creation and cannot overwrite an earlier
failure. They make no global first-attempt claim. The accountable owner's
persistent source/input/case attempt ledger remains authoritative. Original
producer defects, first-run source failures and earlier evidence are retained.
