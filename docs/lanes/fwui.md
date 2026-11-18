# Lane: fwui   (copy to docs/lanes/fwui.md; update in every push)
Owner: Classic FWUI successor (Dot FWUI relinquished writing). Branch: `lane/fwui`. Draft PR: #975. Published head: `90fa6d5f0cefa9913e76a6acc8a4ba9cb57c8613`. Updated: 2026-11-18 10:03 EST.

## Checklist (from CHECKLISTS.md)

A narrow PASS below qualifies only the identified repaired production source. Full lane acceptance remains open; unrun finish-line checks remain FAIL.

| ID | Check | Status | Evidence / remaining qualification |
|---|---|---|---|
| C1 | Exact-head Local gate | FAIL | Integ retains the original 90fa run; completed FREE PLAY and key-sound failures are preserved. No successor full gate yet. |
| C2 | Consolidated ancestry and no loss | PASS | 76040daa remains an ancestor; repairs add no deletion or branch rewrite. |
| C3 | Independent complete screen acceptance | FAIL | FREE PLAY repair received four independent review passes; scoped native images do not establish full lane visual acceptance. |
| C4 | Full hard-limit gate | FAIL | Exact repaired-head gate not yet run. No new forbidden files or runtime paths in these fixes. |
| C5 | All finish-line items closed | FAIL | F1/F4–F7 and final gate/acceptance remain open. |
| C6 | Ready and merged by Integ | FAIL | PR975 remains draft at published90fa; no Ready or merge operation. |
| F1 | All R22 screens and shared glyphs | FAIL | Recovered revision comparison and complete native acceptance remain open; separate reference owner retained. |
| F2 | All-domain geometry | FAIL | Repaired-source caption geometry: 1,530 reached states, 40,967 texts, zero findings; full component/all-domain gate and final acceptance remain open. |
| F3 | Native code-space fit | PASS | Header repair source inc13f85bd6 / unchanged ui21397ecf: independent Elite1044 B, Pro1052 B XIP free; poolmargin780 B. Same stock thresholds. |
| F4 | Pan and battery on all readouts | FAIL | No fresh complete repaired-head gate result. |
| F5 | Pads/LEDs and hold-GLOBE | FAIL | No fresh complete repaired-head host-suite result. |
| F6 | Firmware per-step slider lane | FAIL | Complete scenario/render acceptance remains open. |
| F7 | Genuine Synth One/SuperJ6 pages | FAIL | Genuine Sound controls/runtime admission and complete UI binding remain open. |

## Now / next

- Now: qualify the FREE PLAY negative-input repair and CP/MA key-sound header correction against the actual gate failures.
- Next: consume Integ's terminal record, reconcile the published head/base, deliver only the reviewed repair commits through the existing lane/PR, and obtain the successor exact-head gate.
- Then: continue the shared selector/glyph and broader page work from the accepted base. The separate selector candidate is unpublished and fails stock XIP reserve; its passing framebuffer checks do not authorize delivery.

## Blockers and recovery

- Original90fa gate is still owned by Integ through terminal. Consume complete exit/state/log/status evidence from that owner; do not duplicate or interrupt it.
- Unpublished Sound selector costs64 B versus90fa and fails the1024 B XIP floor (980/988 B free). Preserve it as an unqualified candidate and continue bounded UI code-space work after the current gate repairs.
- Full Synth One/SuperJ6 UI admission depends on genuine Sound-owner controls and resource-qualified runtime; no substitute engine or control surface is accepted.

## Top risks

1. The accepted header repair retains only20/28 B beyond the required XIP reserve; later code must be measured.
2. A green geometry walker is not full page/revision/owner acceptance.
3. Old-owner audio tails, held locks and motion must remain distinct from next-hit selector identity.

## Deletes

None.

## Conflicts noted

The user's current instructions retain the existing branch/PR, assign the active90fa gate exclusively to Integ, prohibit Codex/Actions/devices/main/force-push/PSRAM, and store artifacts/checkpoints in Space/Library. Those instructions supersede older generic workflow text.

### 2026-11-18 original mock recovery and renderer repair

Recovered all 19 omitted PR703 glyph and settings PNGs at their original paths. Every byte count and SHA256 matches the manifest at 64575f86375e7daa9a939de25e6eb2f5047e0665. These are historical mocks. Production comparison uses separately rendered frames.

At 10aaceca the geometry probe stopped before rendering because it still targeted the old relief caption. Updated the probe to instrument the current flat caption. The full geometry subtest then rendered 1,525 states, audited 38,731 texts with zero findings, checked 17,591 content assertions and 249 drum glyph pixel assertions, and passed its negative controls. The 94197fc8 baseline also rendered through the current walker (1,524 states). Independent visual review and the complete lane gate remain open.

- 2026-11-18: Merged #970 (6fa8bb32 → 76040daa); exact fwui gate PASS: 501 host tests, 1,530 geometry states and 46 first-slice screens; broader UI and engine work remains open.

### 2026-11-18 FREE PLAY negative compatibility repair

Integ alone retains the existing exact-90fa gate through terminal. Its [14:50 UTC observation](https://github.com/gobbajogger/DnBOX/pull/975#issuecomment-6982060401) reports the positive FREE PLAY fixture passed, but the historical negative ui.c lacked `physical_stage_label` now required by the drum include. That compiler failure is not an accepted negative. The historical source hash was also unavailable in the successor checkout and GitHub contents API.

Replace only that historical input with a current-source missing-cue mutant. Retain the independent rendered-pixel oracle and wrong-state mutant, including required `FREE_PLAY_MODE_CUE_MISSING` / `FREE_PLAY_MODE_CUE_UNEXPECTED` markers and runtime `FAIL` assertion. No firmware source, control, threshold, image fixture or C oracle changes belong to this repair. The source no longer requires an unreachable historical Git object merely to run the current test.

The separate SOUND selector correction remains an unpublished implementation unit; its candidate resource failures are not accepted or part of this gate repair. The original Integ gate is neither duplicated nor interrupted. Exact repaired-head full gate and independent integration qualification remain required before delivery. No lane completion, native resource change, merge or main write is claimed.

### 2026-11-18 CP/MA key-sound header repair

The [second Integ diagnosis](https://github.com/gobbajogger/DnBOX/pull/975#issuecomment-6982466703) identifies a genuine positive-render failure: the CP stage page uses y26 while the established22-voice cue oracle and redraw mask require y28. An isolated replay reproduced `KEY_SOUND_OFF_STATE_MISSING` plus the C runtime assertion on unchanged90fa firmware.

Move only the SOUND title, conditional KEY SOUND OFF cue and page indicator baselines from26 to28. Keep the panel, physical control captions, graphs and ticks unchanged. Extend the existing positive fixture with alternate-MARACAS cases and actual CP/MA screenshots in both themes; preserve the y28 oracle and allowed-redraw mask byte-for-byte. Separately remove the generic cue and CP-stage cue in current-source mutants; require each intended runtime failure after successful compilation. Persistent mutant directories are reusable, so repeat runs retain fresh source inputs.

Independent native builds preserve Elite1044 B / Pro1052 B XIP free and780 B pool margin. Focused framebuffer positives and both cue-removal negatives pass, including a repeated run over existing output directories. The complete repaired-head gate remains open. Detailed artifacts and checkpoint are maintained on [DnBOX FWUI implementation and qualification](https://chatgpt.com/space/page_800ecfd4f2a08191884949bf6bbee920).

Fresh repaired-source geometry also passes: 1,530 actual rendered states; 40,967 opaque-text audits with zero findings; 17,741 presence assertions with zero missing; 249 glyph-pixel checks with zero missing/damaged. Runtime geometry, presence, pixel and envelope negatives reject their intended damage, and missing/empty audit controls are rejected by the validator. This is the reached-state semantic-caption/presence subtest, not complete R22 design or aggregate gate acceptance.
