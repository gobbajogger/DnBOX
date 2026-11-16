# Firmware UI living status

Updated 2026-11-16 10:48 UTC. Accountable owner: Dot FWUI. Sole UI/app/PR951 writer retained. Integration/Release: repository steward. Sound: sound_engine_qualification. This is the one live UI status file; [finish-line checklist](fwui-finish-line.md) records required pass/fail coverage. Root remains sole roadmap writer.

## Exact source and current action

- Public UI WIP: `fix/r22-drum-glyphs` at `37c3f30354759c4027a01f38453a1461cade2f5e`, tree `285301167d90494e14687a2cc1b67f72ba0a4224`, parent9da1c410. Remote CAS/readback and local raw commit SHA authenticated. First clean exact-head battery68checks and strict held-caption/erased-caption tests PASS; complete stdout/stderr and ledger retained outside the immutable source until the evidence follow-on.
- Existing sole draft PR951 `fix/ui-defect-gate` at `37c780cdf4b18e824c863d7cc2982792882f61cb`. Gate writer explicitly released by Integration to FWUI. No second UI PR. It remains draft/red, not accepted product.
- Live integration target verified at `e742be7bd0d01323c899de6e156f40b85ee451db`, separate from nightly, release and website sources. UI and target diverged from94197fc8. A normal three-way source merge is prepared in an isolated worktree, with UI37c3/e742/gate37c preserved as parents. No landed PR merge, deletion or release. Only common product path ui.c auto-merged; R2/resource/R026 and all21 gate files retained. All recovered raw ancestry commits authenticate; this is not a claim of a full clone.
- Next immutable WIP push by11:04UTC includes compatible composition, reviewed hook extension and R024 review corrections if actual tests permit. Whole all-available-screen WIP remains targeted11:30UTC, with real new-engine states explicitly missing until admitted. Next source after each preservation remains subject to affected tests and independent review.

## References consumed

Canonical `docs/ui/UI-SOURCE-OF-TRUTH.md`, exact2891B/SHA256868957f911acd3d2de67f61db41f2f49a552d0f023d69be67df96a637149add6. Both909+808R22 are shared references: radius9, native four-column tracks, consistent header/footer/type/control units. Improved17:00 glyph drawings retained except requested SuperJ6 WAVE/SUB/LFO RATE redesign. Locked24-page proposal SHA256db826c9cbeb84971f2d030c847043a21749ad4172e30739773494df0c9acca17 and notes1f6b0743bc8ba955a718d2a41421ec410a19cbb5c75ec3b9978b7a533304ed72 were visually inspected. Six-column desktop composition requires an explicit four-column240px adaptation, not pretend numerical equivalence.

RULES(1).md and all24 UI-defect checks consumed. OPEN-ITEMS.md remains NOT_RECOVERED after two Library-title searches; root knows no verified identity. Integration/test owner is checking handoff/repo alternatives. Missing input is not cleared. Direct owner now requests no user progress during autonomous window until12:19UTC except finished lane or actual rule violation; internal coordination continues.

## Actual implementation, tests and remaining gaps

| Area | Actual state and next acceptance evidence |
|---|---|
| Common R22 system | Shared neutral material/radius9/header and control units implemented; Home/FX/Mix/drum mixer body adaptations partial. SEQ/Song/GEN/Global/help/list/SMC portions remain legacy/incomplete. No all-page10/10 claim. Preserve controls, bindings and source-real activity. |
| Drum geometry | Frozen9 independently audited45states/135panels:107→27→4→0 findings, no glyph/value/text intersections; thresholds70%/12% unchanged. Radius4/9 exact-anchor hook extension is the byte-identical accepted delta. New composition still needs fresh actual render/evaluation and all-state coverage. |
| R023 pan | Real L50/C/R50 readouts/markers and normalized-unit exclusion fixed; no DSP/MIDI/save raw-law changes. Focused RED→GREEN, final all-state qualification incomplete. |
| R024 battery | Genuine ADC-smoothed0..3bars;4USB-power and-1unread distinct. No fuel gauge/calibrated SOC, therefore no invented percentage. Exact37c3 focused68checks PASS. Independent review found SMC overlay/peek battery absent and two real long status toasts clipped. Both original failures preserved. Current changed source reserves the same battery in SMC, and wraps complete status text on two native-font lines. Overlay/peek and strict full native-font toast tests now PASS locally; exact new-head/re-review/device checks pending. |
| Held-step controls | Actual NOTE/GATE/ACCENT/SLIDE routing restored. A true NOTE stem/caption collision was found and stem moved below the caption; strict exact-AA font test and erased-label negative PASS. Earlier wrong observer/background and coordinate failures retained, not relabeled. |
| R025 pads/LEDs | Existing behavior retained, current final source/device qualification NOT_RUN. |
| R026 current-instrument clear/copy | Authenticated8a/e742 source retained by normal merge; final combined behavior/anti-revert still required. No silent all-part clear. |
| Genuine SuperJ6/SynthOne | Not admitted in current firmware UI. Old generic Juno and Drone remain active; no rename disguises them. Sound immutable PR952/e6c76e3b supplies64real SJ6 descriptors,150S1 controls,888B standalone SJ6 blob; all SJ6 bindings NOT_ADMITTED. Existing32B cells cannot contain that state. Save-safe new identity/schema, actual graph/getters and stock-memory fit remain required. |
| R029/R037 emulator | Actual host render is not complete interactive/device evidence. Final supported emulator/state inventory NOT_RUN. |
| Whole local/native/device gate | NOT_RUN for combined UI. Old resource four-config proof has only172B pool margin above8192floor; changed UI trial XIP overflow retained. No PSRAM, floor waiver, timing/stack/no-slower/device claim. Independent review required before ready/merge. |

## Evidence, recovery and retained failures

-22c589ef first clean focused tests:6PASS/2FAIL; all raw outputs are committed under docs/ui/evidence/22c589ef. Canonical geometry fixture updates were justified against changed source; they do not erase original failures.
-9da1c410 and37c3f303 source and exact Gitbundles are Library roundtrip byte-verified. Latest bundle `fwui-20261116-1034-37c3f303.bundle`,72316B/SHA256cd0b8683cd74808928f78aadac62c6a1d5c84c45ed99bcd93497fef493c3a699, requires authentic base94197fc8. Separate captures/attempt ZIP86258B/SHA25612b0bf86d6606993d5e68fdd00ba69cb8f733ea7cfde8aa8c49e9d2e47e5137c.
-GitHub safety review rejected generated native PNG publication as raw captures under the older exclusion. No retry, encoding/route workaround or visibility change. Exact PNGs remain recoverable in Library; Git capture delivery explicitly unresolved. Unaffected source/tests/logs continue.
-870constructed captures exist for exact22 only, not newer source or complete independent acceptance. Frozen9 and full source/evidence checkpoints are durable; no unique work discarded.
-First gate fail-open findings R1–R5 and canonical C1/C2 were independently fixed within bounded code scope. That does not pass incomplete product coverage. Portable baseline normal-frame/typed-ID harness supports old e742 without injecting new renderer components; the R22-specific geometry tracer remains unsupported on pre-inc source.

## Guardrails

WIP permission is not merge/release approval or acceptance. No network Git, GitHub Actions/workflows, force/main push, protected-region change, PSRAM, hardware write or branch deletion. Exact changed-head applicable gates, retained-content triple-check and independent before/after render/behavior remain prerequisites to landed merge. No denied runtime/tool action bypass. Only secrets/tokens are excluded by the latest direct owner publication rule, but explicit tool denial still controls its rejected action. All failures/setup mistakes/reruns remain retained and labeled honestly. A required FAIL/NOT_RUN remains an unfinished lane.
