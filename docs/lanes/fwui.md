# fwui — R22 drum-screen checkpoint
Owner: Dot fwui/M2. Base: consolidated `7b17a64e11731de50f4fe46ef7ebe3948a730a10`, incorporating reviewed mixer/FX checkpoint `cb85840d69646e3b08513b9b6a14c43f7db79c7c` without duplicating its memory saving.
Status: drum screen implemented; exact-input fit PASS; host/render checks PASS; full Local gate FAIL; not integrated, no full M2 or device-delivery claim.

## Finished slice
- R22 909/808 voice banks, actual control captions/values, voice pictograms and real 16-step strip.
- Actual hit/idle animation retained for Tune, Pitch/Depth, LPF, Accent and discrete SOUND positions; Shells/Snappy return smoothly. Existing battery/header and control mapping unchanged.
- First-slice capture now emits measured source-closure.json, instrumented closure and actual render/trace receipts; evaluator thresholds unchanged. No nightly baseline regenerated.
- Lossless 9-bit dictionary preserves every authored coordinate and float result. No glyph/control reduction.

## Evidence
`docs/evidence/fwui-r22-drum/`: native logs, exact-input manifest, ELF/bin hashes, geometry/render archive, host checks.
- Coordinate fixture: 1,651 checks PASS, all 1,650 coordinates bit-identical.
- New production dv_glyph live/idle/SOUND tests: 41 PASS, including all Tune/Depth/Pitch branches, Accent and intermediate Shells/Snappy return.
- Capture tooling: seven tests PASS. 45 drum states: evaluator checks 1–4 PASS, zero overlaps. All 45 packed PNGs byte-identical to repaired uncompressed source.
- Native Elite: image551232B; XIP1440B. Pro: image551220B; XIP1452B. Pool335244/344064B; raw free8820B; margin AFTER8192B reserve628B on both. Required XIP1024B / margin512B met.
- Full Local gate attempted: FAIL in isolated non-Git snapshot/environment. This does not satisfy integration gates. Legacy source still requires sound's separate removal composition; no Drone-removal work redone.

## Measured XIP plan
1. Before semantic repair: Elite1496 / Pro1508B free; rejected because live glyph behavior regressed.
2. Restoring behavior costs784B: Elite712 / Pro724B, below floor; not published as qualified.
3. Exact coordinate dictionary reduces data3300→2524B; accessor/alignment costs48B. Net native saving728B; final1440/1452B. Visual/coordinate equivalence verified.
4. Remaining headroom above gate is416/428B. Every further UI composition must remeasure both models. Next optimize duplicate caption formatting/shared rendering, measured against identical controls, before integrating more screens; do not assume standalone M1 fits this image.

## What's left / full lane checks
F1/F2/F7: remaining screen domains, synth pages, overlays and full glyph inventory not qualified; first-slice geometry covers45 drum states only.
F3: this slice fit PASS; full combined M1/M2 pending exact source qualification.
F4: battery intentionally retained at current integration baseline per latest owner correction; pan-wide tests remain.
F5: all pads/LED/hold-GLOBE interactions not fully requalified.
F6: L13 production slider patch preserved separately with45 live/8238 regression checks; BREAK velocity and full composed fit/review remain open.
C1/C3/C6: full Local gate, complete before/after review and authorized integration remain required. No unsupported10/10/all-M2 claim.
Next: compose accepted sound/removal and gate fixes through integration owner, run exact full gate, continue remaining screens/L13 without dropping controls or battery baseline.
