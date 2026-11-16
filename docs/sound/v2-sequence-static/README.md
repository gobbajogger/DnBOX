# Block-scoped sequence admission: component candidate

Only two runtime files change. The public sequence process/can_process boundaries still fully validate. After a successful block admission, SourceScheduler may call a private friend-only path that retains dynamic clock, held-note and event-capacity checks every sample. Static sequence controls are reused only if the actual46-slot smoothing map is disjoint from all56 sequence inputs. Any future overlap falls back to the original full per-sample validation. No fields, DSP arithmetic, note logic, polyphony, tuning or effects change.

The existing serialized, non-reentrant, nonmutating callback contract is essential. Ports.tick is the only in-block parameter writer; current notification callbacks are no-ops. NPO's existing nonpositive-to12 normalization preserves the same static arithmetic domain. Validation at the next block boundary is never cached away.

## Evidence

- Compiler instrumentation of the real tuning_valid function first observed129 calls per128-frame block and failed the intended1-call bound. Candidate normal path now passes1.
- A test-only valid alternative smoothing map assigns a slot to sequence pattern00. Naive reuse failed (1 versus129); the actual-map fallback now passes129. This synthetic map is never a production edit.
- Invalid static input rejects before changing owner/output bytes. Changed controls between blocks get fresh admission. A clock invalidated after one initially valid sample is still rejected by dynamic sample checks.
- Original suite:38 cases,354,907,688 checks,114,196 frames,120,289 state snapshots,1,081 control/note events,0 failures and0 observed runtime heap calls.
- Fresh frontend6,385 changes/957,750 target comparisons/4,096 frames and12-scene576,000-frame C/C++/source comparison remain0 differences/heap. Audio SHA256 remains4b32746b36590998cf7c215ec07d5890d394414e8b2702b7a1a5e847c8ed6b34, matching the independent pre-alias/pre-pack reference.
- Native31 runtime/wrapper/frontend objects plus layout compile. Owner remains7,972B/alignment4; host9,536B. SourceScheduler's unlinked executable sections increase132B; readonly, writable and BSS totals are unchanged. This is not linked application size or S6 fit.

Full694 source/common-FX audio replay is running on the frozen candidate; old factory/timing receipts do not qualify this changed runtime. Independent candidate review is pending. No zero-deadline-miss claim, full sound PASS, native admission or timing fix is made.

## Native receipt recovery

`native-baseline.json` and `native-final.json` losslessly deduplicate repeated dependency hashes. To restore original RESULT.json bytes: take `receipt`; replace each unit's dependency path string with an object containing `path` followed by `sha256` looked up in `dependency_hashes`; encode `json.dumps(receipt, indent=2) + "\n"`. Verify original_bytes and original_sha256. The original final receipt's scope string miscount is preserved, not silently rewritten; native-summary.json gives the verified31+1 count. The baseline is a fresh comparator with the exact old two files restored from521963's verified compiled-input closure, not a claim to recover an old native build.
