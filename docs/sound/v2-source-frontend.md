# Original scalar frontend laws through the C ABI

This adds `dnbox_s1_source_parameter`, `dnbox_s1_source_normalized` and
`dnbox_s1_target` to the pre-FX caller-owned interface. Existing
`dnbox_s1_parameter` remains a raw resolved-physical setter. The source setters
preserve original clamping, tempo/rate dependencies, portamento half-time and
A4 scalar truncation. They do not implement the complete app tuning table,
hold/modwheel, notifications, preset selection or post-FX graph.

The runtime helper is derived from the already authenticated MIT original
frontend fixture. It has a distinct namespace, portable include spelling and
immutable literal friendly-name return, and removes an unreachable diagnostic
print behind the validated public boundary. Original oracle files are unchanged.
The exact source manifest records original and derived bytes. No UI labels or
wire schema changed; these calls are not yet connected to active firmware.

Each source operation uses a serialized transient helper, not retained owner
memory. Native owner remains 7972 bytes/alignment4. Native compiler output shows
248 bytes scratch plus20 bytes saved registers before callees. This is not the
complete call-stack cost. Additional native helper symbols total3414 T+104 W+
1292 r bytes before linking/deduplication; these are not combined-app admission
numbers. All-engine flash/RAM and originalFX remain blockers.

## Tests and exact inputs

`v2-source-frontend-result.json` binds changed files, runtime and adapter source
snapshots, exact command receipt and output hashes. The renderer stages its
wrapper/harness and retained original frontend fixture, checks actual compiler
depfiles against the snapshot and rechecks hashes around compilation/execution.
Six focused closure regressions include changed staged wrapper rejection and
an actual compiler's unlisted include. Eight factory runner regressions include
excluding renderer-only frontend helpers from the factory's separate link.

The source-law C ABI test compares6385 operations and957750 target float words
to the retained original helper, including150 defaults, all six normalized
laws over101 values, five tempos and both sync settings, A4 truncation,
invalid/no-mutation cases and4096 actual output frames. It records zero word
differences and zero control/render heap calls. The existing12-scene576000-frame
pre-FX test still matches its pre-alias/pre-pack output SHA256 exactly:
`4b32746b36590998cf7c215ec07d5890d394414e8b2702b7a1a5e847c8ed6b34`.

Reproduce after restoring the reviewed portable bundle:

```
python3 tools/restore-synthone-test-inputs.py --help
python3 tests/host/run_synthone_renderer.py --source-build <restored> --output <fresh>
python3 tests/host/test_synthone_rebuild.py
python3 tests/host/test_synthone_factory_runner.py
python3 tests/host/run_synthone_factory.py --source-build <restored> --output <fresh-factory> --build-only
```

Build-only factory smoke passes and explicitly records audio NOT_RUN. The older
frozen694-program evidence remains at f50/e454, not silently requalified at this
changed source. Original oracle warnings remain visible (deprecated import and
potential uninitialized private getter path); only the oracle compilation
reduces those known warnings from errors. Runtime compiles with its prior strict
warnings. No full S1/S2, independentFX, complete tails, app tuning, native link,
callback xrun or hardware qualification is claimed.
