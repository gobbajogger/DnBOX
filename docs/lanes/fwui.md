# Firmware UI lane

The FWUI changes are on [draft PR #975](https://github.com/gobbajogger/DnBOX/pull/975), targeting `integration/dnbox-consolidated`. The [exact-head qualification comment](https://github.com/gobbajogger/DnBOX/pull/975#issuecomment-7056161789) identifies the published revision and holds its scoped gate log.

## Stock firmware

Both firmware models were built from [the frozen production commit](https://github.com/gobbajogger/DnBOX/commit/2a348ebde1d9ad17e7437e919ecb7ff107d37f26), input tree `50122c5d95009e0aacef384efad52f1cf0396f31` with AC79 SDK 1.2.14 and JieLi 20260730.1. The actual source and audio-input blobs match the build receipt. Re-linking with a map produced the identical whole ELF for each model, and all 5,188 tracked inputs still matched after both builds. [Stock measurement and input objects](fwui/current/stock-fit.json) · [Production-input equivalence](fwui/current/production-inputs.json) · [Post-build verification](fwui/current/postbuild-source-verification.json) · [Reproduction](fwui/current/reproduce-link-maps.py).

| Stock model | Application image | XIP free | Required XIP | Raw pool free | Margin over 8,192 B reserve | Normal RAM |
|---|---:|---:|---:|---:|---:|---:|
| Elite | 550,760 B | 1,912 B | 1,024 B | 8,972 B | 780 B | 97,156 / 98,304 B |
| Pro | 550,752 B | 1,920 B | 1,024 B | 8,972 B | 780 B | 97,156 / 98,304 B |

## Firmware screen inventory

Each of the 1,712 state IDs has a 240×240 PNG in the repository. The searchable index gives the tar.xz filename, PNG path and decoded RGB digest for every state. The archive covers four battery levels, the running envelope playhead, both kinds of SMC pickup feedback, all 20 edited step-lane combinations, the two post-crash core notices and the safe-mode retry transition. The state captures distinguish complete firmware frames, native display operations with simulated inputs, direct painter previews and partial boot-driver paths. [Screen index](fwui/screens/screen-index.tsv) · [Archive parts](fwui/screens/index.json) · [Reproduction](../../tests/ui_inventory/README.md).

![Exact 240×240 screen captures of battery, envelope playhead, SMC pickup, two edited step lanes and safe-mode retry](fwui/screens/review-preview.png)

The scanner follows firmware source and quoted includes from the production unity build. Stable function IDs use the defining path and C symbol. Of 118 discovered renderers, 98 ran in recorded frames. The other 20 include startup, LCD power and hardware calibration routines, unused helpers and alternative render paths. Display-only captures do not mark native wrappers as executed. [Renderer IDs](fwui/screens/stable-renderer-ids.tsv) · [Remaining source references](fwui/current/renderer-gaps.json) · [Native route limitations](fwui/current/native-gaps.json).

## Checklist

| Item | Status | Evidence and remaining work |
|---|---|---|
| F1 | Open | Every captured state is indexed and archived. Independent acceptance against the R22 reference and shared glyphs remains open. Native startup, hardware tails and alternative routes have the limits listed above. |
| F2 | Verified on captured frames, open across all routes | 1,530 ordinary frames passed 41,884 text-mask, 18,571 content and 249 drum-glyph checks. Another 162 frames passed the final-pixel audit of 3,325 text masks. The tempo popup accounts for all 24 occluded underlay pixels and the main area restores byte-for-byte. The 20 persisted-edit frames passed 376 additional text-mask checks. The hardware-only paths in F1 still lack a complete host-frame result. [Geometry](fwui/current/geometry.json) · [Extended coverage](fwui/current/extended-geometry.json) · [Popup restoration](fwui/current/tempo-layer/layer-proof.json). |
| F3 | PASS | Both stock models exceed the 1,024 B XIP and 512 B pool-margin floors. [Elite linker map](fwui/current/final-elite-link.map.gz) · [Pro linker map](fwui/current/final-pro-link.map.gz). |
| F4 | PASS in software | All 27 admitted numeric pan controls display L50 / C / R50. CLIP, KEYS and pad calibration use the shared header, and both independent SMC headers use the same battery painter. The battery reports four levels and unknown/USB states. Percentage calibration is unavailable in the board API. [Owner and runtime checks](fwui/current/control-tests.json) · [Final focused runs](fwui/current/closure-tests.json). |
| F5 | Focused checks pass, full host suite open | All 7,229 SMK pad/LED scenario checks pass. Hold-GLOBE, copy and the held-step cases pass. The independent full host-suite result is pending. [Focused runs](fwui/current/closure-tests.json). |
| F6 | Partial | On the SMK-37, hold SEQ and tap SCALE to cycle velocity, chance, pitch and pan. Hold SEQ and a step and turn K3 to edit. FM-1 uses SEL and ALGO. Nineteen combinations change and survive project reload. BREAK pitch/pan reads and writes occur within the same audio guard. A runtime regression injects a recording update during lock acquisition. BREAK velocity still needs a genuine per-hit sequencer-to-voice gain contract. The boards expose physical buttons and encoders, with no touch input for the on-screen label. [Edit scenarios](../../tests/host/ui_step_lane_edit_test.c) · [Atomic-edit regression](../../tests/host/ui_step_lane_transaction_test.c) · [BREAK interface](fwui/current/break-velocity-ownership.json). |
| F7 | Open | SMC pickup, pad orientation, hardware calibration and recovery use control labels and status values. The raster text audit covers 885 distinct visible strings across 1,712 states. Fifty cases have exact caption checks. The engine editor still exposes zero Synth One engine controls per part and 16 SuperJ6 controls per part. The isolated Sound runtime renders an audible Synth One INIT (8,192/8,192 frames). Its two exported performance controls are bend and damper. The genuine eight-field oscillator setter remains a separate unapplied experiment, and the complete SuperJ6 editor remains unavailable. [Visible text](fwui/current/text-scan.txt) · [Extended screen text by case](fwui/current/visible-text.tsv.gz) · [Runtime comparison](fwui/current/sound-runtime-comparison.json) · [Parameter binding evidence](fwui/current/synthone-binding.json). |
| C1 | Open | The designated test-and-merge owner runs the complete Local gate and posts its exact-head log. The scoped no-loss and limits log is linked above. |
| C2 | PASS | The lane includes consolidated `76040daa5b52d686a8b7b544296ba5d79610a95d` with zero file deletions. The qualified PR comment links the exact head. |
| C3 | Open | The test-and-merge owner performs the independent before/after 10/10 visual review under Rule 9. The screen PNGs and digests are available above. |
| C4 | PASS | The official limits substep and stock-memory evidence are recorded in the exact-head qualification comment and the map receipts. |
| C5 | Open | F1, F2, F5, F6, F7 and common acceptance still have the explicit items above. |
| C6 | Open | PR #975 is a draft. Integ owns the merge after the three required checks. |

## What's left

Sound must admit the factory control/readback interfaces, especially Synth One parameter storage and render-thread ownership. The referenced historical Synth One port `56079bbd` is a measurement renderer. A current-source host comparison and the separate oscillator experiment identify a path to integrating genuine controls. BREAK per-hit gain must be applied to each triggered voice without changing other voices, live strikes or old audio tails. Independent gate and visual results belong to test-and-merge, and Integ owns the merge.

This checkpoint also retains the earlier physical control and drawing fixes for 808 Accent, 909 bass-drum Tune, six sequencer rates, BREAK groove, Miami wobble, master compression, FX RETURN and HOME SMC pickup. Barlow cap heights come from the original raster before compression. The full corrected-font picture set remains available in the previous checkpoint. [Clock and compressor tests](fwui/current/clock-comp.txt) · [Font regression](fwui/current/font-cap.json) · [Earlier cap comparison](fwui/current/cap-frame-comparison.json.gz) · [Latest changed frame IDs](fwui/current/screen-delta.json).

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
