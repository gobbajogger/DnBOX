# Lane: sound
Owner: Dot sound continuation. Branch: lane/sound-removal-consolidated. PR: #964. Updated: 2026-11-17.

## Status
Local gate: FAIL (sanitizer environment and full review/gate unresolved). Native reserve-correct fit: PASS. Five affected host selections: PASS.

Approved removal reused from7bdc3c78 onto consolidated7b17a64e, initial extraction1246ebf5. Synth One OS11 occupies slot2; old slot2 projects adopt init. The slot is still silent until separate M1 renderer admission. No removal reimplementation, no merge or release claimed.

## Finished corrective moves
- Reconstructed and SHA-verified all4,879 extraction blobs from API/tree evidence.
- Reused independently reviewed no-cut Chicago FX shared arena from cb85840d without mixer/UI changes. Same456 B saving as M2: do not double count.
- Corrected stale preset-navigation assumptions for the approved performance-only singleton init. Base2288 checks PASS; corrected composition2320 PASS. Independent narrow test review PASS; future renderer persistent controls must update this assumption.
- Final source Elite/Pro PADCAL=1 builds with exact SDK214: XIP free15452/15464 B; pool335244/344064 B; raw8820 minus8192 reserve =628 B margin on both. Floors1024/512 PASS. Initial extraction margin172 failed before arena remedy.
- Legacy mapping, SynthOne slot, poly-chord, Chicago FX overlay, preset navigation selections PASS. Overlay81factories/1944blocks bit-exact; retirement/reuse covered.
- Evidence: docs/testing/sound-removal-validation.md and sound-removal-*.log.

## LEFT / next
- original-sound-cell and pattern-preset-notelock LeakSanitizer runs fail under current tracer. Escalated retry also blocked; leak detection was not disabled. Restore untraced test execution and run the full Local gate plus independent screen Review before merge.
- Full gate still contains network Git calls, prohibited by standing workflow. Gate owner coordinates supported API-fed local inputs; this lane has not bypassed no-loss.
- Separate M1 renderer remains held: pinned Elite native diagnostic overflows23092 B before waveform placement; LTO experiment reduces overflow to17752 B, still fails. Full165292 B immutable wave pack has no approved placement (documented free region4096 B); no sample/user/OTA reservation consumed, no engine/FX cuts. Continue source specialization and placement work, then exact combined Elite/Pro checks.
- S1/S2/S3/S4/S5 and complete S6/C1-C6 remain open, not lane completion.

## Deletes
- firmware/src/dsp/drone_string.h: approved removed engine.
- tests/host/drone_slot_test.c: replaced by synthone_slot_test.c.
- tests/host/drone_string_test.c: replaced by legacy slot mapping coverage.
