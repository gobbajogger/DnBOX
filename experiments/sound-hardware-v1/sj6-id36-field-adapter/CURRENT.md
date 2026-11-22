# SuperJ6 field 36: independently verified pre-note binding

The [implementation report](README.md) and `IMPORT-MANIFEST.json` preserve the exact measured adapter, tests and first runner. With explicitly independent envelopes, the adapter binds ID 36 (`neonattack`, layer 1) to the physical DnBOX candidate. Five normalized values matched the independent physical setter across 10,240 stereo frames. The complete part state also matched. Layer 2 remained audible and unchanged. Raw-only, wrong-layer and linear-law controls failed as expected.

The historical `verify.py` uses a Python assertion for mutation-anchor uniqueness and does not reject symlink parents itself. The supported replay entry verifies all recorded core and overlay source bytes, snapshots them into a fresh directory, explicitly checks each mutation anchor, rejects existing/symlink output paths and runs that immutable measured runner in isolated normal Python. Use a fresh output directory:

```sh
python3 experiments/sound-hardware-v1/sj6-id36-field-adapter/check_field.py --repo . --out /tmp/new-sj6-id36-check
```

The final entry was exercised with both `python3 -O` and `PYTHONOPTIMIZE=1`. All four host compilations succeeded; raw-only/fixed/wrong-layer/linear-law run exits were 1/0/1/1. A symlinked output parent also rejected before measurement without changing its sentinel. [Actual replay](evidence/GUARDED-REPLAY.json) · [Independent review](evidence/review/REVIEW.md).

No persistent fields or per-sample work were added. This is one already-decoded, pre-note field applied to the experimental candidate; full preset import, live callback behavior, linked layers, ID 119, original complete-preset audio, target fit/timing and production activation remain open.
