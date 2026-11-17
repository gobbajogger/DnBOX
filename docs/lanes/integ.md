Move: M0 (clean start and green gate)   PR: #956   Head: this commit
LEFT items PASS: 1/2 (L15); failing: L16
Gate at head: NOT_RUN (budget output regression 6/6 PASS; native/environment admission next)
ETA merge: 2026-11-17 22:00 VLAT

## Work
Both start PRs merged; exact START-FW/START-NT are in CONSOLIDATED-STATE.md. Existing gate prerequisite repairs restored separately. Build summaries now print model-attributable XIP free and pool margin above the 8192-byte reserve; existing overflow guards preserved. No battery/device changes.

## Next
Run exact combined gates with connector-pinned base, normal browser sandbox and default sanitizers. Obtain correct native SDK input closure; do not substitute an older SDK. Missing visual baselines/browser checks remain open. Assign product failures to their owning moves, never silently claim PASS.
