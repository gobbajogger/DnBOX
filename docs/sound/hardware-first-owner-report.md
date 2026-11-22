# Sound hardware-first implementation checkpoint

## Decision

This checkpoint accepts a **tested research core and an evidence-backed replacement design**. It does not admit either instrument into production. The strongest measured implementation fits the existing simultaneous RAM allocations, but its complete replacement image still exceeds physical flash and the required flash reserve. Original SuperJ6 and Synth One sound/control equivalence and complete native callback timing remain open.

The current thread is the sole Sound production owner after Dot Sound's explicit freeze and relinquishment. The existing branch is `lane/sound`; the existing draft is [PR #973](https://github.com/gobbajogger/DnBOX/pull/973). The consumed checkpoint archive has SHA-256 `4cc1b62ece0685eaa29b2c7ff86e7b58a4760a50b829bf27a18c38f6ef1bd773`. The handoff head was independently rechecked as `b90faafd2de71063448beea67aa5d22938a5c64b`; live consolidated was `76040daa5b52d686a8b7b544296ba5d79610a95d`.

Latest user direction governs new implementation: use existing efficient DnBOX components and newly written behavior adapters. Original engines and prior source-shaped adaptations are comparison references only. Existing historical reference files are preserved; they are not newly admitted runtime dependencies. No new production branch, main update, force-push, external Codex job, Actions run, PSRAM allocation or device write is part of this work.

## Architecture that was actually built

The C core reuses DnBOX oscillator, float-math, two-pole filter and allocation mechanisms. Each part owns six voices with two independent layers; each layer has separate oscillator/filter history and amp/filter envelopes. Two part instances share no mutable state. The new curved envelope was derived from numeric observations, without giving the implementation worker original engine code.

The fixed part state is 2,264 bytes, including 900 bytes of raw-value storage. The real replacement fixture adds the existing 18-macro facade and stereo high-pass state, giving a 2,356-byte adapter. Together with the preserved 536-byte stock chorus, that consumes 2,892 bytes of the already reserved 3,720-byte backend union. The surrounding 1,464-byte key/source owner remains intact. Both runtime parts therefore remain 10,368 bytes together.

Physical raw/BLEP oscillator selection and linear/squared velocity selection are explicit. These choices prevent a known mismatch from being hidden by one universal setting. They do not make the stock two-pole filter, oscillator sources, lifecycle or FX original-equivalent. In particular, the new candidate places its amplitude multiplier after its stock filter; the preserved Synth One reference evidence calls for VCA-before-filter and a different four-pole transfer.

The prototype stores all 218 finite raw app values losslessly but automatically binds **zero** of those IDs to DSP. Its physical parameter API is separately documented. This is deliberately not an S3 pass.

## Independent behavior evidence

The original comparison component replay reproduced 42 of 42 accepted captures, 942,080 stereo frames at 48 kHz, with zero differing binary32 words. Four deliberately incorrect variants were rejected. This establishes the selected reference component, not the entire original app, all presets or full FX. The preserved original-mode failure after 21 of 32 captures and missing N01 preset qualification remain failures.

For the new core, a separate tester fixed maximum absolute envelope error at 0.001 before comparison. Both supplied recordings pass at their exact timestamps:

| Observation | Comparisons | Maximum error | Result |
| --- | ---: | ---: | --- |
| 10 ms attack, zero hold, 50 ms decay, 100 ms release | 1,024 | 6.9402e-7 | PASS, measured subset |
| Positive hold and release hold | 4,096 | 7.94665e-7 | PASS, measured subset |
| 16-frame time shift | Two fixtures | 0.06372 / 0.02248 | Correctly rejected |
| Linear envelope substitution | Two fixtures | 0.32132 / 0.26942 | Correctly rejected |
| Squared velocity against the captured linear relation | 4,095 usable samples | 0.99213 normalized error | Correctly rejected |

The same recordings were supplied to the implementer for fitting. The independent test validates the fitted subset; it is not held-out evidence for unobserved original behavior or the original 44.1 kHz grid.

The frozen source-derived ADSR zero-attack restart failure remains unclosed. The new amplitude AHDSHR comparison is a different component and does not close the original filter/pitch-envelope transition test.

The independent reviewer found a real oldest-held allocator defect after repeated note retirement and reuse. The writer corrected it; both failing and passing snapshots remain. Five block partitions now produce bit-identical stereo output, part state and render counters. Other checks cover two-part isolation, four independent layer envelopes, sustain release, checked error preservation, finite extremes, zero/one-sample transitions and a complete 16-second attack/release at 192 kHz.

The compact proposal preserves audio and state against the accepted core over 262,144 frames across two parts and four rates. Its separate boundary audit covers 441 cases including adjacent binary32 bounds, signed zeros, subnormals and nonfinite values. This profile saved 152 bytes in the actual native replacement link; disabling diagnostic counters saved nothing further because normal LTO already removed the unused counters.

## Simultaneous memory and actual native links

The baseline and replacement measurements use the recovered JieLi pi32v2 r3 toolchain and actual application link graph at consolidated `76040daa`. They preserve all other existing engines, drums, BREAK, global FX, UI buffers, two runtime workspaces and two selected private-FX owners.

| Quantity | Elite | Pro | Requirement |
| --- | ---: | ---: | --- |
| Stock baseline image | 551,492 B | 551,484 B | Physical area 552,672 B |
| Stock XIP free | 1,180 B | 1,188 B | At least 1,024 B |
| Accepted-core replacement, reachable APIs, `-Oz` | 553,572 B | 553,564 B | FAIL |
| Compact proposal, reachable APIs, `-Oz` | 553,420 B | 553,412 B | FAIL |
| Compact physical overflow | 748 B | 740 B | Must be zero |
| Compact shortfall against required reserve | **1,772 B** | **1,764 B** | Must be zero |
| Ordinary RAM extent | 97,204 B | 97,204 B | 98,304 B |
| Pool, all simultaneous owners | 335,092 B | 335,092 B | 344,064 B |
| Pool raw free | 8,972 B | 8,972 B | Includes 8,192 B reserve |
| Pool margin after reserve | 780 B | 780 B | At least 512 B |
| Further pool growth before gate failure | 268 B | 268 B | Not a new engine allowance |
| XPOOL | 9,216 B | 9,216 B | Fully occupied |

Physical replacement links failed as expected. Enlarged diagnostic-only linker scripts were then used to measure the overflowing image. Those diagnostic outputs are never described as fitting firmware.

The pool includes the occupied 158,400-byte global delay, 66,240-byte canvas, 36,420-byte project/undo stage, 18,316-byte FX bus, 10,368-byte runtime pair, 8,504-byte selected-FX pair, 909/808, BREAK, workspaces and other owners. None was reclaimed or aliased to manufacture a fit. The selected-FX arena holds one chosen algorithm per part; it does not prove a simultaneous original chorus/delay/reverb graph. Stack capacities are known, but new native maximum stack usage is not measured.

## Callback cost: measured and still unknown

The real firmware contract is 512 frames at 44.1 kHz, implemented as two 256-frame renders: **11.609977 ms per DMA half**.

For two candidate parts, 12 active voices and 24 layers, the 512-frame renderer has 6,144 voice-slot visits, 12,288 layer samples, 24,576 envelope-sample evaluations, 2,048 LFO updates and at most 36,864 BLEP evaluations for the saw/pulse blend. These are semantic work bounds, not target cycle counts.

The independent event/control burst includes 32 note events, 436 raw stores, four layer updates, four pedal updates and two velocity changes. The full selected trace performs 168 direct exp2 calls, 112 log2 calls, 208 stage entries and 160 envelope ticks. Public render counters report only 48/48/144/96 respectively. Event work is therefore included in timing and separately traced; render counters are not represented as complete callback accounting.

| Host observation | Median | p99 | Observed maximum |
| --- | ---: | ---: | ---: |
| Independent saw/pulse plus complete selected burst | 311.742 us | 820.857 us | 4,032.101 us |
| Independent sine/triangle, steady | 274.828 us | 1,082.586 us | 9,785.132 us |
| Writer run after native jobs terminated, highest of four scenarios | See receipt | See receipt | 2,486.405 us wall; 2,484.113 us thread CPU |

Both host receipts are retained. Their scenarios and host scheduling conditions differ. Neither is converted into a device margin or worst-case execution-time guarantee.

The measured slice excludes the existing global/private FX, drums/BREAK, other instruments, mix/USB paths, HAL and interrupt interference. The immediate event API has no event cap of its own. Complete callback WCET and xruns remain **unmeasured**. The test package identifies the existing target instrumentation: enclosing `render_block`, TIMER4, `x0x_dbg`, `x0x_worst.tmr` and `audio_xruns`. No target execution was performed.

The independent [stock callback admission audit](../../experiments/sound-hardware-v1/resource_review/CALLBACK-ADMISSION-CONTRACT.md) establishes conditional ingress ceilings of 256 command slots, 64 MIDI packets and 31 physical-key entries per half. Those counts depend on the current native producer arrangement and are not a DSP-event bound: source release, sequencer dispatch, motion, note locks, stale timers and synchronous preset adoption expand work separately. The existing command payload has only 12 value bits, so the probe's 436 raw binary32 stores cannot be mapped unchanged onto it. The selected probe is therefore not the stock worst case. Inter-core waits and a 24 ms fault timeout are further reasons that ingress arithmetic cannot prove the 11.61 ms deadline.

## Mapping and original-control constraints

The SuperJ6 inventory contains 640 presets across 12 banks and 174 distinct serialized names: 172 recognized and two unresolved legacy names. App IDs (218), main-order slots (170), ordinary application entries (167 after exclusions), selected component IDs (52) and the checklist's unverified 64-control catalog are distinct sets.

Stored values include gain 1.32, waveform gain 1.001, route selector 70, pad selector 9 and a negative sequence value. Blanket normalization would corrupt them. Both layer-sound selectors are explicitly zero in all 640 presets; default substitution would change their meaning.

Known scalar differences include original attack up to 16 seconds versus stock 3.002 seconds, original free LFO 0–16 Hz versus stock 0.1–8.1 Hz, and original PWM 3–50% versus the stock macro/render limits. The recorded amplitude component uses AHDSHR; active original filter/pitch-envelope responses and all routing/FX laws remain unqualified.

The fresh Synth One mapping audit verifies 150 control IDs, 46 smoothing memberships and 144 ordered writes for each of 694 factory programs. The old dry-screen subset contains **zero intact factory presets**: 480 disable bandlimiting, 431 also use LP, 58 also have oscillator 1 at saw, three also mute oscillator 2, and none also mute the sub. A fresh routing probe confirms the candidate's postfilter amp/velocity application and rejects the legal S1 22,050 Hz cutoff. S1 waveform endpoint 1 is saw, while candidate endpoint 1 is square; the candidate also applies voice/channel normalization absent from a direct scalar forwarding scheme. New numerical S1 ADSR observations show that a nominal 50 ms attack peaks at frame 6,092 at 44.1 kHz, about 138.14 ms, with per-sample updates. It cannot inherit the SJ6 AHDSHR grid unchanged.

The source-free [S1 behavior packet](../../experiments/sound-hardware-v1/mapping/synthone_followup/implementer_brief/BEHAVIOR-BRIEF.md) includes 100 cutoff targets, 264,600 new reference envelope observations and eight deliberate negative checks. Existing Chicago MP4 and Miami MS20 components offer state/preparation patterns, but require an exact stock-code provenance allowlist and independent transfer checks before reuse; no equivalence or flash saving is inferred from their names.

For Synth One, the 694-program reference replay and 45 focused tests are preserved. The 45 tests were rerun successfully at the exact handoff source. The original-code renderer is comparison-only under the latest direction. Its historical 3,602 wall and 2,629 CPU misses in 520,500 standalone callbacks remain a failure. The common production Synth One slot is still silent. No fresh whole-program Synth One pass is claimed from this new core.

## Admission state and next concrete work

| Gate | Current state | Evidence needed to close it |
| --- | --- | --- |
| Clean new implementation provenance | PASS for this research core | Keep original sources out of implementation dependency graph |
| Captured SuperJ6 amplitude subset | PASS | Additional held-out captures, original 44.1 kHz and lifecycle behavior |
| S1 / S2 Synth One sound and timing | FAIL | New DnBOX-based implementation, matched original topology and complete program A/B |
| S3 / S4 SuperJ6 controls and sound | FAIL | Actual control bindings, source/oscillator and FX matching, authenticated preset captures |
| S5 bank/control application | OPEN | Independent complete application mapping, not just field inventory |
| S6 stock hardware fit | FAIL | Recover at least 1,772/1,764 bytes for this partial core, then include remaining control/FX costs |
| Complete native callback and stack | OPEN | Bounded event policy and whole callback measurements with simultaneous owners |
| Common Local gate and independent integration review | OPEN | Exact-SHA before/after results; no production admission from this package |

The next implementation unit must be chosen from the measured dependency closure, not another history-codec sweep. The current largest replacement costs include render, the legacy macro facade, allocation and envelope preparation. Share existing math/control code where it preserves behavior and measure the resulting complete image. Additional filters, control catalogs, sources and FX must be budgeted before feature acceptance. Reclaiming unrelated live buffers or silently reducing existing functionality is not a fit strategy.

The independent [narrow-adapter decision](../../experiments/sound-hardware-v1/resource_review/NARROW-STOCK-ADAPTER-DECISION.md) recommends testing new envelope behavior directly within the stock product interface. The native linker already inlines much of the generic facade: removing an API layer is not a measured multi-kilobyte saving. A useful control-path change is to prepare pitch only on pitch edits; the current generic layer update otherwise recalculates twelve voice increments through the two-layer facade. The whole-image gate still permits only 156/164 bytes of net growth from stock, so this proposal requires a new complete link and behavior comparison before it can be called feasible. The 900-byte raw owner already occurs once per part; moving two copies outside the accounted union would consume another 1,800 bytes and fail current RAM margins.

See [the research package](../../experiments/sound-hardware-v1/README.md) for source, tests, exact snapshots, command logs and receipt paths.
