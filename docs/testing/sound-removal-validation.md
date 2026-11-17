# Sound removal composition validation

Base: consolidated `7b17a64e11731de50f4fe46ef7ebe3948a730a10`.
Approved removal donor: `7bdc3c786e0e4e0ef4155b486a31ddad59d74ec9`.
Initial extraction: `1246ebf55d85eeaba98ebf0b598a456eedab1807`.
All 4,879 tracked extraction blobs were verified against the GitHub API recursive tree. No network Git or Actions were used.

## Corrective changes

The extraction reused the approved product/test blobs without redoing removal. The before/after run found a test assumption that no longer describes the approved replacement: every bank was assumed to have editable persistent parameters and multiple factory presets. The Synth One init placeholder has only two performance controls and one factory preset. Exact base passes 2,288 preset-navigation checks; the extraction fails at the edited-state assertion.

The corrected test keeps persistent-edit and adjacent-preset coverage for every applicable bank. It asserts that Synth One performance changes do not mark the preset edited, and adds both-direction no-op and deferred-reload checks for its single init. Corrected test: 2,320 checks PASS before the arena composition; final composition evidence is in the accompanying logs.

The no-cut Chicago FX arena change is reused from independently reviewed `cb85840d69646e3b08513b9b6a14c43f7db79c7c`, without its mixer/UI changes. The backend and inline FX share the lifetime arena of mutually exclusive engines. The factory/retire/reuse comparison checks 81 factories and 1,944 blocks bit-exact. This is the same shared saving used by M2 and must not be counted twice in combined budgeting.

## Native inputs and initial fit

- Toolchain: jieli-linux-toolchains-20260730.1, pi32v2.
- SDK: AC79NN_SDK_V1.2.14_2026-10-13. Pinned uboot SHA256 `64ccc63e4df3238046fadb200f1b134311d8fe6ec6e3ccb8c8b81379231b867e`.
- Elite and Pro; PADCAL=1; no device operations.
- Extraction1246: XIP free15,548/15,560 B; pool335,700/344,064 B. Raw free8,364 B minus8,192 B reserve =172 B margin. Both FAIL512 B floor.
- The old gate print labels raw free as margin; qualification here always subtracts the reserve.

## Remaining gates

The original-sound-cell and pattern-preset-notelock sanitizer executions fail because LeakSanitizer cannot run under the available tracer. An escalated retry of original-sound-cell encountered the same restriction. Leak detection was not disabled and these tests are not claimed PASS. Full Local gate and independent screen Review remain unsatisfied; no merge or delivery is asserted.

Synth One is still the intentional silent init placeholder in this removal composition. The playable renderer is separate M1 work, not included or qualified by these measurements.

## Final composition measurements

Elite: XIP free15,452 B, image537,220 B. Pro: XIP free15,464 B, image537,208 B. Both: pool335,244/344,064 B, raw8,820 B minus8,192 reserve =628 B margin. Native floors PASS. Same shared456 B saving as M2, not an additive saving.

Final affected selections: legacy-slot2-map, synthone-slot, poly-chord-integration, chicago-fx-overlay, preset-navigation-noop PASS. Preset navigation2,320 checks. Independent test-change review found the singleton/performance-only contract correct and non-SynthOne checks retained; it did not independently rerun the tests. Full gate remains FAIL/unqualified for the reasons above.
