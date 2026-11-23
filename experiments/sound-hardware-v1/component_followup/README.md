# Independent Synth One envelope implementation

The reusable C component is `firmware/src/dsp/dnbx_s1_envelope.c` and its public
header. It is independently derived from numeric observations, with the
existing DnBOX finite-validation/ownership conventions. It is not an original
engine import, a modification of the separate SJ6 AHDSHR, or a production
renderer. The recorded contract is retained in `IMPLEMENTER-CONTRACT.md`;
its initial “implement next” wording is history, now implemented by this unit.

One state is 12 bytes. Retention coefficients are prepared externally and
latched at each stage entry; sustain is a live target. Edits/events apply before
the sample. The attack threshold uses the previous output. Retrigger retains
the current level. Separate multiply/add operations preserve binary32 behavior
under the documented no-fast-math/no-FMA contract. Release tails are not
truncated and this API does not declare a note retired. Invalid prepared
parameters reject without modifying prior values.

From the repository root:

```sh
X0X_TESTS='synthone-envelope-dnbx fastmath fastmath-ubsan' X0X_TEST_JOBS=1 sh tests/run_tests.sh
```

The selected dispatcher returned zero for all three tests. This is a scoped
run; its Pro package skip and lack of a full suite remain explicit in the log.
The envelope tests compare three static plus seven live scalar recordings,
379,288 float words total, with exact equality and zero gain/time adjustment.
Both binary fixture and metadata hashes are pinned. Five behavior tests cover invalid controls,
independent owners, reset and the edit at the Attack/Decay boundary. Two further
integrity tests reject changed metadata and truncated retained PCM (seven total).

Independent C review rejected four concrete mutations: relatching a running
time coefficient, snapping sustain, resetting level on retrigger, and checking
the attack threshold after recurrence. The last mutation initially survived;
a dedicated exact-binary boundary case now rejects it. That boundary case is
an API invariant, not another original-app recording. The review also retained
independent irregular grouping, guard and endpoint evidence for the unchanged
C implementation. `C-COMPONENT-REVIEW.json` records the final scope and limits.

The saved earlier C core fails all 81,920 compared live words in five cases;
`c-component-red.log` records comparison of those retained actual outputs.
It does not claim that the old C binary was executed again in this turn.
The source-free Python hypothesis, numeric proposal receipt and failed
alternatives are retained separately; they are not production DSP.

The final simplification pass used three independent reviewers. It centralized
binary32 format/width handling, used the public reset API, selected fixtures by
stable identity, ensured setup-failure cleanup and deferred diagnostics until
an exact comparison fails. The C sample routine was unchanged. Final hashes
and focused outcomes are in `IMPLEMENTATION-RECEIPT.json`.

Target coefficient generation, target arithmetic/denormals, the full voice
signal path, controls/presets/FX, complete memory fit, callback deadlines and
listening remain open. Production Synth One remains silent. The component is
not wired into a production build or selector, and no full gate is claimed.

## Input-integrity corrections before publication

The final package review reproduced three harness defects: Python optimization
could remove bounds-test actions inside `assert`; unpinned observation metadata
could repeat one recorded case while omitting others; and an optimized retained
baseline comparison could accept truncated PCM. Initial negative receipts are
preserved. Portable checks now use explicit runtime requirements, pin the three
metadata files before case selection/compilation, and reject incorrect PCM
lengths explicitly. The same negative cases reject with optimization off and on.
The selected dispatcher and both portable checks then passed with Python
optimization enabled. `HARNESS-FINAL-RECHECK.json` records independent validation.
These fixes preserve all DSP arithmetic and original numeric observations.
