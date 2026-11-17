## Capture current source without a pre-existing profile

`capture_source.py` records the actual input closure, verifies its isolated copy,
creates a measured `SOURCE-PROFILE.json` only in that copy, and invokes the normal
first-slice renderer. It does not manufacture receipts for an older run.

```sh
python tests/ui_defects/first_slice/capture_source.py \
  --source /path/to/frozen/source --output /path/outside/source/new-capture
python tests/ui_defects/first_slice/evaluate.py \
  --render-dir /path/outside/source/new-capture/render \
  --output /path/outside/source/new-evaluation.json
```

An optional `--expected-head FULL_SHA` checks local HEAD. It does not establish
that the working tree is clean: actual working-tree bytes remain the authority.
The capture records HEAD/status when available, and uses a closure hash when Git
metadata is absent. Existing profiles are preserved separately. Existing output
is never overwritten; source mutation during copying fails closed.

The render directory includes source-closure.json, instrumented-closure.json,
instrumentation.json, run.json, traces.json, report.json, actual log and PNGs.
The unmodified evaluator checks receipt identity and trace/log consistency.
The 70% panel bounding-box and 12% empty-rectangle thresholds are unchanged.
Coverage is still 45 drum states only, checks 1–4 plus overlaps; other domains,
hardware and human scoring remain NOT_RUN. A detector pass is not all-UI acceptance.
