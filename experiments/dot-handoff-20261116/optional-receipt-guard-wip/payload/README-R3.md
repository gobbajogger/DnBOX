# Optional local receipt aid, corrected R3

This is a disjoint successor to immutable R2, not an adopted scheduler, mandatory gate, permission decision, retry or delivery service. It remains read-only with no external calls.

The independent R2 review found empty/missing ID acceptance, unbound supplied occurrence/future completions, discarded error wrappers and an unhashable outcome crash. Its exact eight strict cases were rerun against R2 copied into this successor before edits: RED retained, seven failures and one schema error.

R3 requires nonempty string IDs on both sides, binds a supplied occurrence to the actual run in timezone-aware time, rejects future/before-attempt completion bounds, propagates error wrappers rather than falling back through them, and returns invalid structure for malformed receipt/outcome types. A plain named collection is permitted only when it is not an error/service wrapper; CLI errors exit2 with zero classifications, never partial success. This does not authenticate the evidence or report product acceptance.

All original25 tests, independent15 tests and16 consequence regressions pass in normal/optimized modes (56 each). The independent historical snapshots were classified without service calls: old nulls stay unmatched; stale retry output stays rejected; matching QA/throughput results stay documentary-only, with product/live source/liveness unproved and delivery unknown. These are historical fixture tests, NOT subsequent actual scheduler consumption.

Same-reviewer affected recheck and real later workflow consumption remain pending. No schedule/prompt/runtime hook was changed, no new gate is introduced, and ordinary authorized program work never waits on this optional aid.
