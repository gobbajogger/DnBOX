# Firmware UI lane

Branch `lane/fwui`, draft PR #975, targeting `integration/dnbox-consolidated`. The independent test-and-merge owner handles the aggregate gate and visual review.

## Current stock and screen evidence

The code and generated resources in this revision were built from Git tree `3a08f435bb9a7d1643f21b41ed4cbfaadb9b2493`. The two final firmware images use stock SMK-37 memory. Both genuine linker-map relinks reproduced the original ELF byte for byte. The prior baseline measured 1,028 / 1,036 B XIP free. [Current link maps, inputs and logs](fwui/current/) · [Baseline](fwui/stock-04b5/).

| Stock model | XIP free | XIP floor | Pool free | Margin above 8,192 B reserve | Normal RAM used |
|---|---:|---:|---:|---:|---:|
| Elite | 2,276 B | 1,024 B | 8,972 B | 780 B | 97,204 / 98,304 B |
| Pro | 2,288 B | 1,024 B | 8,972 B | 780 B | 97,204 / 98,304 B |

The control fixes preserve pattern/kit ownership while displaying 808 Accent firing and 909 BD Tune, actual sequencer clock rates, BREAK groove, Miami wobble, and the effective master compression curve. FX RETURN uses the established K7/K8 send controls. HOME faders and encoders now show SMC pickup in the SMC grid. Explicit PAGE/PAUSE remains the route to a peer peek. The lossless font encoding preserved the tested pixels and advances. The host draw benchmark recorded higher CPU cost, with the slowest full page at 2.26 ms. [Timing by page](fwui/current/host-font-timing.json).

| Check | Result at this checkpoint |
|---|---|
| F1 | FAIL. The firmware-derived inventory contains 118 stable renderer IDs and 1,689 distinct 240×240 frame captures. The run reached 98 renderer functions and all four targeted pickup/playhead branches. Twenty functions remain unobserved, including hardware entry points and unused helpers. The metadata distinguishes complete UI frames, RAM-backed board painters and display-only boot slices. Independent R22 review remains open. [Stable renderer IDs](fwui/screens/stable-renderer-ids.tsv) · [Frame archive index](fwui/screens/index.json) · [Verification](fwui/screens/verification.json). |
| F2 | FAIL across all domains. The ordinary walker passed all 1,530 states, 18,571 content checks and 249 drum glyph checks, including its negative controls. It caught and led to fixing 144 overwritten BREAK captions. Additional board-only domains still need geometry audits. [Geometry](fwui/current/geometry.json). |
| F3 | PASS. Both stock models meet the two memory floors. [Builds and linker verification](fwui/current/stock-fit.json). |
| F4 | Pan and battery host checks passed, including the 10,975-check pan test, 9,898 battery checks and the overlay battery suite. All-header coverage still needs the board screen inventory. [Control tests](fwui/current/control-tests.json). |
| F5 | The real pad/LED scenario passed 7,229 checks on 909, 808 and BREAK. Hold-GLOBE and copy semantics passed. Full host suite at the final PR head remains open. |
| F6 | The four real-data step lanes run in V_SEQ. Hold SEQ and tap SCALE on SMK-37 (SEL on FM-1) to cycle. The host scenario passed 1,427 checks and covers keyboard ownership and four pattern windows. BREAK has no admitted step velocity. Literal label touch has no input path on these boards. |
| F7 | FAIL. The production Synth One renderer produces zero nonzero stereo frames on the held-note test. Neither editor has an editable Synth One engine control. The compact SuperJ6 slot has only 16 controls on each part. Sound owns the genuine bindings in #973. The visible-text scan also found explanatory copy on the help cards and empty song row. [Rendered text](fwui/current/text-scan.txt). |

C2 passes. Consolidated `76040daa5b52d686a8b7b544296ba5d79610a95d` is an ancestor and the branch deletes no files relative to it. C1, C3, C4, C5 and C6 remain open. The final Local gate, independent four-size screen review, lane readiness and integration merge remain with their respective owners. To open a frame, look up its stable ID in the archive index or metadata. Extract the indicated tar.xz part with tar. The PNG members are the native 240×240 framebuffer pixels. The current firmware, generator and model build inputs match the preceding qualified revision. The next deliverable is the extended board geometry replay and the remaining input and caption repairs.

## Archived qualification history

## Checklist (from CHECKLISTS.md)

A narrow PASS below qualifies only the identified repaired production source. Full lane acceptance remains open; unrun finish-line checks remain FAIL.

| ID | Check | Status | Evidence / remaining qualification |
|---|---|---|---|
| C1 | Exact-head Local gate | FAIL | Original90fa terminal FAIL: 499 PASS, 4 FAIL, 0 NOT_RUN; both exits1, GATE_RETURNED, authentic dnbox-gate failure59267455179. All four original failures have reviewed scoped repairs. Integ alone owns the active e437 full gate; its last consumed observation has359 PASS and gate-trap FAIL without terminal. |
| C2 | Consolidated ancestry and no loss | PASS | 76040daa remains an ancestor; repairs add no deletion or branch rewrite. |
| C3 | Independent complete screen acceptance | FAIL | Original correction reviews and the bounded KIT review complete with zero unresolved findings. KIT48 states,24 matched frame pairs and runtime wrong-association negative pass; complete lane visual acceptance remains open. |
| C4 | Full hard-limit gate | FAIL | Exact repaired-head gate not yet run. No new forbidden files or runtime paths in these fixes. |
| C5 | All finish-line items closed | FAIL | F1/F4–F7 and final gate/acceptance remain open. |
| C6 | Ready and merged by Integ | FAIL | PR975 remains draft; reviewed corrections are delivered through the existing lane branch. Integ alone qualifies the successor and controls Ready/merge after required checks. |
| F1 | All R22 screens and shared glyphs | FAIL | Recovered revision comparison and complete native acceptance remain open; separate reference owner retained. |
| F2 | All-domain geometry | FAIL | Repaired-source caption geometry: 1,530 reached states, 40,967 texts, zero findings; full component/all-domain gate and final acceptance remain open. |
| F3 | Native code-space fit | PASS | KIT source inc1894cb94 / unchanged ui21397ecf: independent Elite1028 B, Pro1036 B XIP free; poolmargin780 B. Stock XIPfloor1024/poolfloor512/reserve8192 unchanged. |
| F4 | Pan and battery on all readouts | FAIL | No fresh complete repaired-head gate result. |
| F5 | Pads/LEDs and hold-GLOBE | FAIL | No fresh complete repaired-head host-suite result. |
| F6 | Firmware per-step slider lane | FAIL | Complete scenario/render acceptance remains open. |
| F7 | Genuine Synth One/SuperJ6 pages | FAIL | Genuine Sound controls/runtime admission and complete UI binding remain open. |

## Now / next

- Now: preserve the published KIT successor04b5c849 while Integ owns its exact resource/review/full-gate qualification. The historical e437 gate is terminal FAIL with502 PASS, one executor gate-trap FAIL and zero NOT_RUN; no original failure is rewritten.
- Next: retain the independently reviewed list-feedback and REVERB-return implementation as a separate local follow-on on this lane. It is excluded from published04b5 and cannot advance the remote while Integ qualifies that head. Reconcile against fresh accepted consolidated before the next coordinated publication.
- Then: complete the remaining SOUND/HOME/BREAK compositions and actual owner/state qualification. The prepared selector remains separately preserved. Missing originals and full-engine runtime admission remain explicit; this writer neither runs a duplicate aggregate gate nor marks Ready or merges.

## Blockers and recovery

- Original90fa gate has terminated. Its complete v0 archive, manifest, exits, logs and authentic status were consumed and verified; successor exact-head qualification is the remaining delivery gate, owned solely by Integ.
- The prepared selector initially failed stock XIP at980/988 B. A measured private read-only classifier pointer interface recovers64 B, yielding1044/1052 B and780 B pool margin; this separate source unit still requires reconciliation and exact-head acceptance after current repair integration.
- Full Synth One/SuperJ6 UI admission depends on genuine Sound-owner controls and resource-qualified runtime; no substitute engine or control surface is accepted.

## Top risks

1. The minimal KIT correction retains only4/12 B beyond the required XIP reserve; every subsequent candidate needs exact stock measurement.
2. A green geometry walker is not full page/revision/owner acceptance.
3. Old-owner audio tails, held locks and motion must remain distinct from next-hit selector identity.

## Deletes

None.

## Conflicts noted

The user's current instructions retain the existing branch/PR, assign the active e437 gate exclusively to Integ, prohibit Codex/Actions/devices/main/force-push/PSRAM, and store artifacts/checkpoints in Space/Library. Those instructions supersede older generic workflow text.

### 2026-11-18 original mock recovery and renderer repair

Recovered all 19 omitted PR703 glyph and settings PNGs at their original paths. Every byte count and SHA256 matches the manifest at 64575f86375e7daa9a939de25e6eb2f5047e0665. These are historical mocks. Production comparison uses separately rendered frames.

At 10aaceca the geometry probe stopped before rendering because it still targeted the old relief caption. Updated the probe to instrument the current flat caption. The full geometry subtest then rendered 1,525 states, audited 38,731 texts with zero findings, checked 17,591 content assertions and 249 drum glyph pixel assertions, and passed its negative controls. The 94197fc8 baseline also rendered through the current walker (1,524 states). Independent visual review and the complete lane gate remain open.

- 2026-11-18: Merged #970 (6fa8bb32 → 76040daa); exact fwui gate PASS: 501 host tests, 1,530 geometry states and 46 first-slice screens; broader UI and engine work remains open.

### 2026-11-18 FREE PLAY negative compatibility repair

Integ owned the exact-90fa gate through terminal. Its [14:50 UTC observation](https://github.com/gobbajogger/DnBOX/pull/975#issuecomment-6982060401) reports the positive FREE PLAY fixture passed, but the historical negative ui.c lacked `physical_stage_label` now required by the drum include. That compiler failure is not an accepted negative. The historical source hash was also unavailable in the successor checkout and GitHub contents API.

Replace only that historical input with a current-source missing-cue mutant. Retain the independent rendered-pixel oracle and wrong-state mutant, including required `FREE_PLAY_MODE_CUE_MISSING` / `FREE_PLAY_MODE_CUE_UNEXPECTED` markers and runtime `FAIL` assertion. No firmware source, control, threshold, image fixture or C oracle changes belong to this repair. The source no longer requires an unreachable historical Git object merely to run the current test.

The separate SOUND selector correction remains an unpublished implementation unit and is excluded from this gate repair. Historical candidate resource failures remain recorded. Exact repaired-head full gate and independent integration qualification remain required before merge. No lane completion, merge or main write is claimed.

### 2026-11-18 CP/MA key-sound header repair

The [second Integ diagnosis](https://github.com/gobbajogger/DnBOX/pull/975#issuecomment-6982466703) identifies a genuine positive-render failure: the CP stage page uses y26 while the established22-voice cue oracle and redraw mask require y28. An isolated replay reproduced `KEY_SOUND_OFF_STATE_MISSING` plus the C runtime assertion on unchanged90fa firmware.

Move only the SOUND title, conditional KEY SOUND OFF cue and page indicator baselines from26 to28. Keep the panel, physical control captions, graphs and ticks unchanged. Extend the existing positive fixture with alternate-MARACAS cases and actual CP/MA screenshots in both themes; preserve the y28 oracle and allowed-redraw mask byte-for-byte. Separately remove the generic cue and CP-stage cue in current-source mutants; require each intended runtime failure after successful compilation. Persistent mutant directories are reusable, so repeat runs retain fresh source inputs.

Independent native builds preserve Elite1044 B / Pro1052 B XIP free and780 B pool margin. Focused framebuffer positives and both cue-removal negatives pass, including a repeated run over existing output directories. The complete repaired-head gate remains open. Detailed artifacts and checkpoint are maintained on [DnBOX FWUI implementation and qualification](https://chatgpt.com/space/page_800ecfd4f2a08191884949bf6bbee920).

Fresh repaired-source geometry also passes: 1,530 actual rendered states; 40,967 opaque-text audits with zero findings; 17,741 presence assertions with zero missing; 249 glyph-pixel checks with zero missing/damaged. Runtime geometry, presence, pixel and envelope negatives reject their intended damage, and missing/empty audit controls are rejected by the validator. This is the reached-state semantic-caption/presence subtest, not complete R22 design or aggregate gate acceptance.

### 2026-11-18 CP/MA footer compatibility repair

Integ's 15:13:17 UTC observation reported 349 completed PASS and four FAIL without terminal exit files. Two newly identified positive failures, `ui-drum-single-lane` and `ui-drum-curve-axes`, share the CP/MA step row's y214 position. Both established independent oracles require y212. An isolated snapshot of the reviewed header repair reproduced the first mismatch at x9/y212 and the axes composed-step runtime assertion before any production edit.

Change only `dv_voice_steps(T_808,D8_CP,200-off,214-off,1)` to use `212-off`. Keep identity y200, strict length bounds, all sixteen absolute step labels, KIT, physical controls, graphs and actual-owner live state. Correct the newer stage-row test's mistaken y214..<218 range to y212..<216; leave both older independent C oracles and the mutation scripts unchanged.

Actual native GREEN: the existing single-lane harness passes270,616 checks, and curve-axes passes34,294 positive checks plus its preserved missing-curve negative. The stage wrapper passes the fixed attack/curve-law, audio-owner epoch and stage-row checks with all four existing mutants. Independent binary replays confirm all five negative controls exit1 at their intended runtime assertions after successful compilation. Four fresh CLAP/MARACAS dark/light framebuffer images were independently inspected; the continuous footer, identity, controls and owner graphs remain readable.

Independent stock Elite/Pro builds for this exact footer source exit0 with XIP1044/1052 B free and poolmargin780 B, unchanged from the header-only baseline. Native disassembly changes one immediate without increasing instruction size or the affected frames. This is scoped source/runtime/visual/resource qualification; full exact-head gate, complete R22 acceptance, whole-stack and hardware proof remain open. Evidence and recovery artifacts are on the existing FWUI implementation Page linked above.

### 2026-11-18 terminal90fa evidence consumed and correction delivery

The original exact90fa gate completed at15:37 UTC with499 PASS,4 FAIL,0 NOT_RUN across503 host records. Both gate/run exits are1 and run.state is GATE_RETURNED. The authentic dnbox-gate failure is status59267455179; head90fa6d5f, tree918ac4d9 and base76040daa were unchanged. The full terminal archive v0 (`terminal-90fa-fail.zip`,44853 bytes, SHA256 `075d0d0140b9d28af65d98114e10463f74f20c150333c882d228db3298c9db90`) and all31 manifest payloads were verified. [Original terminal report](https://github.com/gobbajogger/DnBOX/pull/975#issuecomment-6983377341) and [Integ ownership confirmation](https://github.com/gobbajogger/DnBOX/pull/975#issuecomment-6983426330) retain Classic FWUI as sole correction writer and Classic Integ as sole successor gate owner.

The reviewed repairs correspond exactly to terminal failures: current-source FREE PLAY negative compatibility; CP/MA header y28 with preserved cue/redraw oracles and alternate-MA coverage; CP/MA footer y212 with both older independent oracles unchanged. The newer stage-row oracle is aligned to that established contract. All controls, flat panels/radii, graph width and native owner semantics remain unchanged by these corrections. No files are deleted.

Local review anchors are76a57af,3f78164 and325368b. Authenticated API publication may produce different commit IDs because its commit metadata is server-generated; every published tree must match its reviewed local tree. Exact local/published mapping, live head/tree/base readback, full review artifacts, native measurements and original terminal evidence are maintained on the existing FWUI implementation Page. A matching tree is source fidelity, not a successor full-gate PASS.


### 2026-11-18 Edition 2 adoption and truthful KIT section

Consumed completed reference handoff v1 (SHA256 d9aa533cf8fa0fc56afe5988e868b8370c13adbb66cef3346424c48ce4ce984d) and exact ZIP v2 (SHA256 222b4cf357e0b1ca5153ba3d7785e84f58aca909c9966fb9cfb9370cc93d1372). All9622 current checksum payloads verify; the older PACKAGE-MANIFEST has300 stale entries and is retained as a metadata discrepancy. Independent source/no-loss maps retain all51 normal compositions,393 control occurrences,45 list schemas/408 rows/91 frames,32 target families/84 banks and8 boards/4 overlays/110 state contracts. Full maps and explicit missing originals/observers remain on the existing implementation Page; inventory coverage is not runtime acceptance. The separate reference owner remains authoritative.

[Integ's refined KIT finding](https://github.com/gobbajogger/DnBOX/pull/975#issuecomment-6985959901) retains the current flat geometry and restrained selected ink; only SOUND2/2 on an active KIT page is wrong. [FWUI coordination](https://github.com/gobbajogger/DnBOX/pull/975#issuecomment-6986154892) confirms this bounded correction and the immutable active gate head. Change only the CP/MA title to page==0?SOUND:KIT. Preserve all parameter owners, physical controls and values, full-row envelopes, key cue y28, identity and16 steps y212. No old panel, fill, color or radius is restored.

The independent real-framebuffer regression first compiles unchanged e437 and fails24 KIT associations of48 states at its C assertion. The candidate passes10628 checks, both palettes/choices/pages, actual encoder navigation and held/released input. A freshly compiled current-source wrong-association mutant fails all24 intended markers at runtime. The complete wrapper passes twice over the same persistent output. All24 before/after PNG pairs are identical outside x11..50/y28..39; SOUND frames are fully identical. Eight primary frames were independently and root-inspected. Existing footer, curve and key oracles remain unchanged. The new wrapper is registered once and serialized as classG because it regenerates shared host inputs.

Independent stock native Elite/Pro builds exit0 at1028/1036 B XIP free,780 B pool margin and8192 B pool reserve. KIT costs16 B versus e437; affected local frame allocations are unchanged. These are direct measurements, not whole-stack, timing or device proof. Three simplification passes found no worthwhile change. Completed independent ce-code-review run kit-e437-20261118-independent reports no actionable findings; no aggregate gate acceptance follows.

The e437 full gate remains owned solely by Integ; at12:32:59 EST it had359 PASS and one gate-trap failure caused by executor placement beneath /tmp. The original failure and this active run are preserved. Do not publish this successor until terminal is recorded and exact head/base are rechecked in coordination. Keep selector WIP and separate whole-page list/caption work outside this narrow delivery. No files are deleted. Exact source, test, native, review and recovery receipts are maintained on [the FWUI implementation Page](https://chatgpt.com/space/page_800ecfd4f2a08191884949bf6bbee920).


### 2026-11-18 KIT publication and prepared whole-page follow-on

[Integ recorded e437 terminal and released only the bounded KIT successor](https://github.com/gobbajogger/DnBOX/pull/975#issuecomment-6986875406). All503 host records completed:502 PASS,1 unchanged gate-trap executor-path FAIL,0 NOT_RUN. The authentic failure status is59281879061. Elite/Pro, flashmap, budget, UI-defects and drone passed. The historical FAIL is preserved. The new raw terminal archive remains to be consumed here when its exact durable version is delivered; primary owner records and authentic status are retained separately.

[Published KIT readback](https://github.com/gobbajogger/DnBOX/pull/975#issuecomment-6987047092): existing lane/fwui and draft PR975 head04b5c8495c5ae4df91e3c2afd06cc81f35c077b8, tree0bfb78f8a15f0eeef079ea32661f03aa271c6376, parent e437, actual base76040daa. The authenticated API changes author/committer metadata, mapping reviewed local0b4ff454 to04b5 with identical tree, parent and complete message bytes. CAS expected e437 and force=false were used. Independent audit confirms all5071 remote entries, five changed paths, two added tests, no deletions and5066 unaffected parent entries. Local0b4 and all12 selector WIP files remain recoverable exactly. Integ retains all independent acceptance, full-gate and integration ownership.

This separate prepared change restores actual SCALE cues on action/category list rows, respecting physical CLIP ownership. GLOBAL PARTON keeps its existing stripes, pictograms and switches and renders ON/OFF through the same descriptor owner. Native baseline RED had204 expectations/142 missing/224 preserved; candidate GREEN has204/0/224. Three compiled current-source negatives reject missing cue22, wrong part owner80 and inverted state160. All44 matched frames have zero changes outside the measured cue/value bands. ABOUT50 and real physical PARTON72 checks pass, as do existing settings763 and list70. The final wrapper clears stale receipts and owns/kills/reaps its compiler process group on timeout; two full repeat runs and a genuine owned-child timeout qualification pass. Independent ce-code-review is complete without actionable findings.

REVERB now exposes existing REV RETURN on K7 and DLY RETURN on K8 while retaining K5 DECAY, K6 TONE and every DELAY control. The list builder explicitly keeps the returns in their original MORE positions, preserving all11 unique FX controls and exact12-row order. Native tests pass468 checks and capture96 real frames. All48 DELAY frames are identical; REVERB changes stay within new controls and the established contextual popup. Three freshly compiled current-source negatives reject swapped owners, missing front slots and list reordering. The final wrapper's two persistent-output repeats and owned-child timeout qualification pass. Independent ce-code-review is complete without actionable findings; the96 fresh combined frames match the preserved green frames exactly.

Both new tests are registered once in shared-generator classG. No old test oracle or threshold is weakened. The exact KIT+list+REVERB source has stock XIP1060/1068 B free, poolmargin780 B and reserve8192 B; all floors remain1024/512. A separate actual stock measurement of selector+KIT+list+REVERB also gives1060/1068 and780, establishing measured net selector composition cost0 B. All5080 snapshot paths retain exact bytes/modes after build. Five composed positive scenarios pass and render205 frames; independent unit negatives remain separate. These receipts do not establish a whole-call-stack, timing, hardware, complete UI or aggregate gate PASS.

Independent SOUND/BREAK mapping identifies concrete remaining geometry work: generic footer is bypassed; SOUND navigation captions must follow ordinary/held/PATCH/pending input ownership and cannot overwrite CP y28/212. BREAK count/page text shares the PENDING/LIVE/MORE band, and the reference STEP instruction at238 collides with current lock notches238..239. Do not hide existing status, assignments or locks to insert text. HOME simultaneous matrix plus five independent live lanes remains a separate composition. All12 edition2 original/observer/runtime gap groups remain open with exact per-key records; inventory coverage does not mean per-control runtime acceptance. Full reports, native receipts, original failures, exact patches and recovery remain on the existing implementation Page.
