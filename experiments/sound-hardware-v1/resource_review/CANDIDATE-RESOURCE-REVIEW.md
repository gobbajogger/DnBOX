# Fresh DnBOX candidate: native resource and replacement review

## Result

**The candidate fits the existing melodic-part state allocation, but every measured replacement fails the stock Elite/Pro flash gate.** The best measured composition is the separate compact validation variant at `-Oz`: **553,420 bytes on Elite and 553,412 bytes on Pro**. Physical XIP capacity is 552,672 bytes. These images exceed that capacity by **748 / 740 bytes** and miss the required 1,024-byte free-space reserve by **1,772 / 1,764 bytes**.

These are real Juno-slot replacement resource fixtures, with the other engines, drums, BREAK, existing selected effects, global effects, buffers, and target memory regions retained. They are not production backend changes. They do not implement the full original SuperJ6 control graph, the Synth One renderer, or the remaining fidelity requirements. Any additional implementation must be budgeted on top of these results.

All native compiler and linker jobs are terminal. No device, target executable, emulator, original application binary, or network Git command was used. No production repository or builder-owned candidate file was modified.

## Exact inputs and source boundaries

The complete firmware baseline is commit `76040daa5b52d686a8b7b544296ba5d79610a95d`. Its separate baseline review verified **379 source/input files against their exact Git blobs with zero mismatches** and built stock Elite and Pro with the recovered official JieLi toolchain and SDK. The baseline source and resource receipts are in:

- [RESOURCE-REVIEW.md](sandbox:/workspace/scratch/67e685478953/superj6_research/resource_review/RESOURCE-REVIEW.md)
- [native-baseline-source-manifest.json](sandbox:/workspace/scratch/67e685478953/superj6_research/resource_review/native-baseline-source-manifest.json)
- [native-linked-ledger.json](sandbox:/workspace/scratch/67e685478953/superj6_research/resource_review/native-linked-ledger.json)

The measured frozen candidate is stored under `dnbx_core_7161c21e6d9d773c/src`:

| Input | SHA-256 |
|---|---|
| Candidate C | `9cdcea0b90b691d05cd07306433b9b4cf42c023bf419a251d7ff436fa13487fa` |
| Measured candidate header | `4bd92cc4971eb4ba03befb1cd3851900e7d69f47b3e5099f2f4b2c68c08d2f3c` |
| Combined snapshot identity | `7161c21e6d9d773cb7f1311be33ec4dc4d59cac74fb76a14b5115fee2364341f` |
| Later documentation-only header | `a863f6b04f65202a3ba7f5640b4dc89e0730a15c096894bf9d914caaf7c8671c` |
| Separate compact-profile C | `3d8f898428b297478abb35692de56952451caed38cc02db53e16a19a54a7408b` |
| Separate compact-profile header | `86acaf17ee9c48dac151919098ab12de3acf457fab2bd9fdf57ac966f36c0962` |

Native preprocessing confirmed that the later documentation-only header changes only blank lines in preprocessed C. The measured declarations and implementation remain identical. The compact profile is a distinct source variant; it is not silently substituted into the frozen candidate.

The earlier `dcabfab3c585bcd4` and `12306973f1a85235` snapshot directories are historical intermediate receipts. Their sizes must not be attributed to the final rank-corrected C.

The isolated replacement tree changes exactly four baseline files and adds two candidate files. **375 of the 379 baseline files remain byte-identical.** The changed files are `juno6.c`, `juno6.h`, `original_runtime.c`, and the isolated copy of `tools/build.py`; the added files are `dnbx_core.c` and `dnbx_core.h`. Source hashes and a diff audit are preserved in `replacement-PROVENANCE.json` and `replacement-native/SOURCE-DIFF-AUDIT.json`.

## What the replacement actually executes

The resource fixture replaces the obsolete stock Juno oscillator/envelope/filter/allocator implementation with the new candidate. Actual runtime init, note-on, note-off, sustain, all-off, active-voice observation, envelope observation, control updates, and stereo render calls reach the candidate. Both layers are rendered for each of its six voices.

The fixture keeps a facade for the stock firmware's existing 18 normalized macros. Existing stock Juno parameter laws drive the candidate's physical parameters within its supported bounds. For example, cutoff remains `35 + 12000*x*x`; pitch bend remains four semitones across the normalized range; stock pulse-width requests are clamped to the candidate's supported duty range. This facade is an explicit resource-testing choice, not a claim that those 18 macros implement the original application's 218 controls. The candidate's raw-control storage remains present in state.

The existing stock chorus implementation is copied byte-for-byte, with source SHA-256 `587bcbfa10b2c9949777f292a4d8e9ea649a8d508ce327ab4b360466f704b3ca`. Its owning runtime, parameter updates, delay history, and processing call remain. Stereo high-pass state and the existing macro facade are accounted for inside the Juno backend member. The shared global effects and the Chicago/Miami selected effects are unchanged.

The runtime calls the candidate's stereo output directly into the existing left/right output buffers. It does not allocate a second render buffer or pretend that a discarded output channel is a complete stereo integration. Only the Juno observation and render branch in `original_runtime.c` changes. The Synth One branch remains the baseline silent keyed placeholder and is **not qualified by this experiment**.

A focused host smoke test exercised two independent instances, six voices and two layers per instance, 512 rendered frames per instance, all 18 macro bounds, note/sustain/all-off, finite output, state independence, and the preserved chorus. It passed. This checks that the replacement facade actually calls working code; it is not a full firmware, original-sound, or timing acceptance test. Receipt: `replacement-native/SMOKE.json`.

## Native build method and flash ledger

The exact target is `pi32v2`, CPU `r3`, with `-mfprev1`, `-ffp-contract=off`, `-fno-fast-math`, function/data sections, and the official LTO/link policy. There is no `-ffast-math` or `-ffinite-math-only`. The strict floating-point contract is material to the compact variant's ordered-comparison validation.

The complete official compile closure was rebuilt for both Elite and Pro after changing the Juno header. This prevents stale layout assumptions in unchanged translation units. The `-Oz` comparison changes only the candidate and facade compilation; unrelated engine units retain the official optimization policy.

Each physical-map link was attempted first. It failed at the actual XIP boundary. A separately named **DIAGNOSTIC-OVERFLOW** linker script enlarges only the diagnostic map so the overflowing ELF can be inspected. These diagnostic ELFs are not stock-compatible images and were not packaged or executed as firmware. Image size includes linked load addresses, copied RAM code/data, alignment, and the four-byte end marker. Consequently, the complete image deficit is four bytes larger than the linker-only overflow printed in some logs.

| Composition | Elite image | Pro image | Elite physical free | Pro physical free | Elite shortfall from 1,024-byte reserve | Pro shortfall |
|---|---:|---:|---:|---:|---:|---:|
| Exact stock baseline | 551,492 | 551,484 | 1,180 | 1,188 | Pass, 156 bytes extra margin | Pass, 164 bytes extra margin |
| Frozen candidate, replacement, reachable APIs, `-Os` | 553,812 | 553,804 | -1,140 | -1,132 | 2,164 | 2,156 |
| Frozen candidate, replacement, reachable APIs, `-Oz` | 553,572 | 553,564 | -900 | -892 | 1,924 | 1,916 |
| Frozen candidate, replacement, all 24 candidate APIs retained, `-Os` | 555,716 | 555,708 | -3,044 | -3,036 | 4,068 | 4,060 |
| Frozen candidate, replacement, all 24 candidate APIs retained, `-Oz` | 554,820 | 554,812 | -2,148 | -2,140 | 3,172 | 3,164 |
| Compact validation variant, `-Oz`, counters enabled | 553,420 | 553,412 | -748 | -740 | 1,772 | 1,764 |
| Compact validation variant, `-Oz`, counters disabled explicitly | 553,420 | 553,412 | -748 | -740 | 1,772 | 1,764 |

“Reachable” is the complete code closure reached by the actual stock-slot facade. It permits normal section garbage collection and LTO specialization. It does not mean that missing original controls have been implemented for free. The all-API variants retain every candidate entrypoint, including raw admission, scalar-law helpers, and oscillator diagnostics, so their cost cannot disappear through an artificial unused-core link.

The compact rows retain the new four-byte `dnbx_diagnostic_counters_enabled` capability query. `DNBX_DIAGNOSTIC_COUNTERS=0` is explicit; the variant rejects non-null work counters atomically with an unsupported status. Its builder-supplied equivalence receipt is copied beside the native results. The native reviewer observed **152 bytes of net image saving**, including the query. Counter-off produces **no additional image-size saving** in this real replacement because LTO already specializes the normal render path for `work == NULL`. The on/off ELF hashes differ even though the image lengths agree.

The earlier additive experiment retained the entire obsolete Juno implementation plus every candidate API. It measured 557,332 / 557,324 bytes at `-Os`, and 559,860 / 559,852 bytes at `-O2`. Those results are conservative additive bounds; they are not replacement feasibility results.

## Simultaneous memory ledger

All values below are measured with the 32-bit native compiler or the complete linked ELF. Host `sizeof` values are not substituted for target values.

| State component | Native bytes | Ownership |
|---|---:|---|
| One envelope | 36 | Per layer voice, two envelopes |
| One layer voice | 92 | Oscillator/filter/envelope state |
| One voice, two layers | 192 | Six per candidate part |
| One layer and parameters | 100 | Two per candidate part |
| Complete candidate part | 2,264 | Includes 218 raw floats and 28 presence-bit bytes |
| Optional diagnostic work counters | 112 | Caller owned; not allocated by the replacement |
| Stock macro facade plus stereo HP state | 92 | Added around the candidate in the existing backend |
| Candidate plus facade | 2,356 | `juno6_t` in the fixture |
| Preserved stock chorus | 536 | Per part |
| Actual active Juno backend member | 2,892 | Candidate, facade, HP, and chorus together |
| Existing backend union | 3,720 | Unchanged; still accommodates other engine members |
| Existing complete runtime part | 5,184 | Backend plus source/key/stale-event ownership |
| Both runtime parts simultaneously | 10,368 | Unchanged allocated pool storage |
| Existing selected FX, one part | 4,252 | Unchanged |
| Both selected FX owners | 8,504 | Unchanged allocated pool storage |
| Existing workspace, one part | 5,120 | Unchanged |
| Both workspaces | 10,240 | One in pool, one in XPOOL; not reclaimed |

The Juno member leaves 828 bytes inside each existing union. That is not new global free memory: the union remains 3,720 bytes because other selectable engines need its allocation. Adding two standalone candidate instances elsewhere would consume another 4,528 bytes and is not what this measured replacement does.

| Whole-image allocation | Baseline | Every replacement / compact variant | Capacity / remaining margin |
|---|---:|---:|---|
| `.data` | 600 | 600 | Included in ordinary RAM extent |
| `.bss` | 96,596 | 96,596 | Included in ordinary RAM extent |
| Ordinary RAM extent, including alignment | 97,204 | 97,204 | 98,304 capacity; 1,100 free |
| `.pool` | 335,092 | 335,092 | 344,064 capacity; 8,972 raw free |
| Pool after 8,192-byte safety reserve | 780 | 780 | Required extra margin 512; 268 bytes above that gate |
| `.xpool` | 9,216 | 9,216 | Fully occupied |
| `.c1stack` | 4,608 | 4,608 | 4,352 user + 256 supervisor capacity |
| `.ram_sink` | 1,024 | 1,024 | Unchanged |
| `.ram_text` | 2,708 | 2,708 | Unchanged |
| `.c1_text` | 608 | 608 | Unchanged |
| `.noinit` | 308 | 308 | 1,024 region capacity |
| `.ble_ram`, Elite / Pro | 3,888 / 3,884 | 3,888 / 3,884 | 4,096 region capacity |

The occupied global delay history (158,400 bytes), UI pixel allocation (66,240), project staging (36,420), global effects (18,316), drum/BREAK owners, undo state, and the two melodic workspaces remain in the ledger. No shared buffer, stack reservation, flash boundary, selected engine, or effect is removed to manufacture headroom. Immutable constants and lookup tables are included in linked `.text`; the compact Elite `.text` is 549,500 bytes, followed by the unchanged copied code/data and end marker.

## Helper closure and stack evidence

The standalone frozen native object has 5,960 bytes of text plus 20 bytes of immutable data at `-Os`, or 8,490 plus 20 at `-O2`. Its only unresolved helpers are `memcpy` and `memset`, which already exist in the complete firmware. Every diagnostic full link has no unresolved symbols and passes the existing symbol scan for soft-double helpers. No 64-bit integer arithmetic helper is introduced by the optional counters.

The real `-Oz` reachable closure shows these useful costs:

| Function / inlined closure | Frozen bytes | Compact bytes where changed |
|---|---:|---:|
| Stereo Juno render, including candidate render | 978 | 978 |
| Juno control facade, including candidate validation/set paths | 952 | 746 |
| Juno note-on, including candidate allocation | 460 | 460 |
| Envelope stage entry | 464 | 464 |
| Candidate outlined exp2 body | 146 | 146 |
| Wave shape | 184 | Unchanged |
| Envelope tick preparation | 124 | Unchanged |
| Stage settling | 112 | Unchanged |
| Pitch increment | 76 | Unchanged |

The candidate's 146-byte outlined exp2 body is additional to pre-existing stock static exp2 bodies. Log2 work is inlined in its stage-entry function, while the baseline also contains a 154-byte outlined log2. This is concrete duplication worth reviewing, but it does not establish a safe deduplication saving or close the remaining 1,772-byte gate deficit. No global helper linkage or numerical implementation was changed in this review. Detailed symbols, addresses, and sizes are in each `SYMBOL-SIZES.csv` and `RESULT.json`.

Linked native disassembly gives fixed-frame/direct-call bounds for the candidate portion of the call graph:

| Entry | Frozen `-Os`, reachable | Frozen `-Oz`, reachable | Compact `-Oz`, either counter profile |
|---|---:|---:|---:|
| Stereo render | 224 | 236 | 236 |
| Parameter/sustain facade | 312 | 328 | 324 |
| Note-on facade | 204 | 216 | 216 |
| Preserved chorus | 56 | 60 | 60 |
| Envelope observation | 60 | 60 | 60 |

These include native saved registers and fixed local/outgoing argument storage along the deepest recognized direct call path. They exclude the caller's engine/runtime frames, interrupt entry and nesting, other engine paths, and concurrent core activity. They are not hardware stack watermarks or whole-callback stack qualification. Core 0's reserved user/supervisor stack capacities remain 10,496 / 7,936 bytes; core 1's user stack remains 4,352 bytes. The immutable allocation and a local call bound do not establish actual maximum stack use.

## Callback cost boundary

The actual target callback is `render_block` in `firmware/src/app/main_fm1.c`: 512 frames at 44,100 Hz, split into two `engine_render(256)` calls. Its elapsed deadline is **11.609977324 ms**; the diagnostic integer budget is 11,609 microseconds. The measured region also includes USB audio tap, master volume, and USB output mixing. DMA-half advancement drives the actual late/xrun counter.

For two candidate parts with six active voices and two layers each, one 512-frame half visits 6,144 voice slots, processes 12,288 layer samples and 24,576 envelope samples, and advances 2,048 layer LFO samples. The maximum saw/pulse crossfade path evaluates three BLEPs per layer sample, or 36,864 evaluations per half. These are semantic work counts, not target instruction counts or cycles. Stage settling is bounded by six transitions per call; event/configuration paths also perform bounded work, including pitch exponentials and the rank-compaction loops.

The optional work struct counts render-path work only. It omits note, pedal, layer-configuration, and scalar-helper calls outside render. The independent acceptance worker directly observed this omission in its burst fixture. Counters therefore cannot stand in for total callback cost. Neither native compilation, helper sizes, static stack bounds, nor host timing prove the complete target deadline under drums, BREAK, both melodic parts, all required effects, USB/radio activity, UI contention, and cache/interrupt interference.

Existing target evidence points are `x0x_dbg.max_us`, `last_us`, and `late`; debug builds also have `x0x_worst.us`, `over90`, `over100`, `wait`, `tmr`, and `nsub`. `fm1_alnk0_irq` increments `audio_xruns` if the free DMA half changes while rendering. TIMER4 provides the existing 24-tick-per-microsecond elapsed clock. The optional performance-cycle helper explicitly says its counter behavior has not been verified on hardware. No target measurement was performed here.

## Receipts and reproduction

Canonical experiment directory:

`/workspace/scratch/67e685478953/superj6_research/resource_review/dnbx_core_7161c21e6d9d773c`

| Artifact | Purpose |
|---|---|
| `CANDIDATE-PROVENANCE.json` | Frozen candidate source hashes and capture paths |
| `HEADER-DOCUMENTATION-DELTA.json` and `.patch` | Header-only documentation change verification |
| `native/RESULT.json`, `COMMANDS.json`, `ANALYSIS.json` | Standalone ABI, additive bounds, helpers, stack |
| `prepare_replacement.py` | Creates the isolated real replacement tree from the exact baseline |
| `replacement-PROVENANCE.json` | Adapter and changed-file hashes, preserved chorus identity |
| `replacement_native.py` | Full native closure rebuild and eight physical/diagnostic links |
| `replacement-native/RESULT.json` | All eight exact images, sections, symbols, ABI sizes |
| `replacement-native/ANALYSIS.json`, `SYMBOL-SIZES.csv` | Stack paths, code closure, sizes |
| `replacement-native/SOURCE-DIFF-AUDIT.json` | 375 unchanged baseline files, four changed files, two additions |
| `replacement_smoke.c`, `replacement-native/SMOKE.json` | Focused executable facade smoke test and command |
| `compact_variant_native.py` | Frozen compact variant, explicit profile, four same-image comparisons |
| `compact-variant-native/PROVENANCE.json`, `RESULT.json`, `COMMANDS.json` | Compact inputs, native ABI, physical failures, exact image sizes |
| `compact-variant-native/ANALYSIS.json`, `SYMBOL-SIZES.csv` | Compact stack and helper closure |
| `compact-variant-native/builder-EQUIVALENCE.json` | Builder-supplied behavioral equivalence receipt, separately attributed |
| `REVIEW-FILE-MANIFEST.json` | Hashes for this experiment's preserved files |

The scripts resolve the sibling `native-baseline-76040` tree and `build_inputs/extracted` toolchain/SDK relative to their experiment location. Recovery must preserve that directory relationship or update the explicit input locations. The original native input archives and their checksums are documented in the baseline report; no installation is needed beyond their recorded extraction.

Reproduction keeps the physical-map failures as failures. It must not substitute the diagnostic enlarged map, omit the required free-space reserve, allocate candidate state outside the measured union without accounting for it, or infer full SuperJ6/Synth One admission from this limited stock-slot bridge.
