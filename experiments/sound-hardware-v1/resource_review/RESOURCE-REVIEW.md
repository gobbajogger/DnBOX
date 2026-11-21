# Hardware-first Sound resource review

## Decision supported by this review

The current stock DnBOX baseline links on both Elite and Pro, with very little growth capacity. At consolidated commit `76040daa5b52d686a8b7b544296ba5d79610a95d`, the freshly built Elite image has **1,180 bytes of XIP free** and the Pro image has **1,188 bytes free**. Their pool margin is **780 bytes after the existing 8,192-byte reserve**. The required additional margin is 512 bytes, so permitted incremental growth is only **156 bytes of flash on Elite and 268 bytes of pool** before those gates fail. XPOOL is completely occupied.

This is an actual native compile/link result for the existing application. It does **not** include a functioning common-runtime Synth One, a complete original-faithful SuperJ6, new component admission, a full Local gate, or hardware timing. The existing Synth One branch of `original_runtime_render()` zero-fills both outputs. The SUPERJ6 label selects a compact six-voice Juno-style engine with 18 parameters and four factories.

The promising direction is a common implementation built from existing firmware primitives with independent per-voice and per-part state. The baseline has a 3,720-byte backend union per part that is already reserved even when compact Juno uses only 888 bytes. That creates potential **replacement capacity inside existing allocations**, not new free RAM. All required concurrent voices, layers, modulation, FX and tails must fit and be exercised before treating that capacity as available.

## Provenance and observed execution

Live GitHub reads verified:

| Reference | Exact commit |
|---|---|
| `lane/sound`, open draft PR #973 | `b90faafd2de71063448beea67aa5d22938a5c64b` |
| `integration/dnbox-consolidated` | `76040daa5b52d686a8b7b544296ba5d79610a95d` |
| `lane/fwui` at inspection | `90fa6d5f0cefa9913e76a6acc8a4ba9cb57c8613` |

The consolidated and Sound tips are divergent; the comparison showed three consolidated-side commits and 51 Sound-side commits. The source snapshot used for this native baseline is consolidated `76040daa`, not an unrecorded union of those branches.

The archived integration playback closure was originally at `f753443cf31bf912356b27df022f951b8c82fcd8`. Its matching files were reused only after Git-blob verification. Thirteen changed files were refreshed from the current immutable GitHub ref, and missing native-only HAL, linker, build and mboot headers were restored. The final **379-file source/input manifest has zero Git-blob mismatches** against `76040daa`; see `native-baseline-source-manifest.json`. Generated assets and runtime versions are recorded separately.

Native inputs:

| Input | SHA-256 |
|---|---|
| `jieli-linux-toolchains-20260730.1.tar.xz` | `a1545cddbc451a06bac43eb5cd96d831470c42a97c5c0b1f98879a20fc7647bd` |
| `fw-AC79_AIoT_SDK-AC79NN_SDK_V1.2.14_2026-10-13.zip` | `929ec0b652c1e246aa066c5ff4bc6068e1fd145501adce30187b8497b93276b6` |

The compiler identifies as JieLi Clang 4.0.1. The actual production build script selected pi32v2 r3, `-mfprev1`, `-ffp-contract=off`, its existing per-unit optimization levels and existing LTO policy. No arithmetic flags or thresholds were loosened. SDK hashes and all 20 locked hardware settings passed the existing checks. The builds reported no linked software-double routines. Host tools are GCC 14.2.0, Python 3.12.14, Pillow 13.0.0 and FreeType 2.14.3.

Commands, stdout/stderr, ELF symbol tables, section tables and hashes are retained in this directory. Initial missing-input attempts are retained rather than relabelled as successful builds. No device operation or target binary execution occurred.

## Simultaneous physical memory ledger

### Linked region totals

| Region / quantity | Elite | Pro | Limit / meaning |
|---|---:|---:|---|
| Load image, including copied RAM code/data | 551,492 B | 551,484 B | 552,672 B app code area |
| XIP free | 1,180 B | 1,188 B | Must remain at least 1,024 B |
| `.text` including immutable constants | 547,572 B | 547,564 B | Already included in image |
| `.data` | 600 B | 600 B | Within ordinary RAM |
| `.bss` | 96,596 B | 96,596 B | Within ordinary RAM |
| Ordinary RAM extent, including alignment | 97,204 B | 97,204 B | 98,304 B; 1,100 B free |
| `.pool` | 335,092 B | 335,092 B | 344,064 B |
| Pool raw free | 8,972 B | 8,972 B | Includes required 8,192 B reserve |
| Pool margin after reserve | 780 B | 780 B | Must remain at least 512 B |
| `.xpool` | 9,216 B | 9,216 B | 9,216 B; zero free |
| Core 1 stacks | 4,608 B | 4,608 B | 4,352 B user + 256 B supervisor |
| `.ram_sink` | 1,024 B | 1,024 B | Reserved Bluetooth probe area |
| `.ram_text` | 2,708 B | 2,708 B | Copied code; also occupies image |
| `.c1_text` | 608 B | 608 B | Copied code; also occupies image |
| `.noinit` | 308 B | 308 B | 1,024 B reserved region |
| `.ble_ram` | 3,888 B | 3,884 B | 4,096 B reserved region |

The core 0 user and supervisor stack windows are separately reserved by `firmware/app.ld`: **10,496 B user** and **7,936 B supervisor**, each next to a 256-byte guard. These are capacities, not fresh measured stack high-water marks. The older engine comment mentioning a 2,816-byte core 1 stack is stale; the current assembly reserves 4,352 bytes. Neither spare Bluetooth space, stack reservations nor guard bands are general-purpose synth memory.

The `.data` and `.bss` symbol lists contain aliases. Summing all symbols would double-count them. The ledger uses address-interval unions and the linked section sizes. `.pool` has 335,088 named bytes plus four bytes of alignment; XPOOL's named allocations exactly equal its size.

### Pool owners that coexist

| Owner | Bytes | Lifetime / restriction |
|---|---:|---|
| Global delay `dly_buf[79200]` | 158,400 | Existing send/tape/ping delay history; not free private-FX storage |
| Canvas `cv_px` | 66,240 | UI rendering; remains part of the product workload |
| `project_stage` | 36,420 | Persistent undo shadow and exclusive project-transfer staging |
| Global `fx` | 18,316 | Reverb, delay and master/send state, including its embedded histories |
| Two `original_bass` runtime parts | 10,368 | 2 × 5,184 B; independent backend and key/source ownership |
| Two selected `original_fx` owners | 8,504 | 2 × 4,252 B; independent selected algorithm state/history |
| 909 | 7,612 | Current drum backend |
| 808 | 3,680 | Current drum backend |
| BREAK | 6,772 | Current sample/decoder/voice state |
| BREAK side state | 4,392 | Separate required state |
| Part A runtime workspace | 5,120 | Existing scratch; scheduled sharing with 808 is deliberate |
| Undo object | 4,252 | Includes its 4,096-byte history |
| Master | 664 | Includes stereo look-ahead histories |
| Other owners and alignment | 4,352 | Controls, LEDs, queues, scope, tuning, UI and remaining padding |
| **Total** | **335,092** | All remain allocated simultaneously |

XPOOL is **4,096 B of stereo buffers for the two synth parts plus 5,120 B of part B workspace**. Moving a new part's independent scratch into XPOOL would overflow it. The existing two-workspace arrangement is necessary when the scheduler puts parts on separate cores; it cannot be replaced by a single global scratch buffer without proving exclusion and preserving the job schedule.

The immutable drum sample payload alone is 55,275 bytes (ride 23,500; crash 21,650; hi-hat 10,125). Existing tables, fonts and code are already counted in the linked image. Sharing immutable data/code can avoid new copies, but does not turn the existing payload into free flash.

### Actual target layouts

`resource_layout.c` was compiled by the recovered native compiler. `resource_layout.json` comes from its object data, not host `sizeof` assumptions. Target pointers and floats are four bytes; doubles are eight.

| Type | Native bytes |
|---|---:|
| `juno6_voice_t` | 36 |
| `juno6_t` | 352 |
| `juno6_chorus_t` | 536 |
| Juno branch total | 888 |
| `original_runtime_backend` | 3,720 |
| `original_runtime_part` | 5,184 |
| Per-part key/source/ownership overhead outside backend | 1,464 |
| `original_runtime_workspace` | 5,120 |
| `original_chicago_fx2_compact` | 4,252 |
| Selected FX cold owner handle | 20 |
| `chi_fx2_insert_arena` | 4,084, inside a 4,096 B arena |

For a proposed common backend, the static state increment is at least:

`2 × (align4(max(3720, new_S1_state, new_SJ6_state)) − 3720)`

plus any increase in per-part adapters/patches/FX, global metadata, scratch and alignment. This formula applies only if each engine kind is mutually exclusive within a slot and its lifecycle fits the existing union. It does not authorize overlapping audible tails or concurrent FX to alias. A chorus + delay + reverb graph needs all its live histories at once. Cold engine replacement currently retires/clears the old backend; that transition must be matched to the required original behavior before relying on union reuse.

## Reuse inventory

| Existing component | Useful mechanism | Limits that need explicit comparison |
|---|---|---|
| `firmware/src/dsp/fastmath.h` | Existing float-only exp/log/pow/tanh/sin/cos, phase wrapping, clamps; outlined shared helpers avoid repeated text | Approximation domains and float rounding are part of the sound; prepare coefficients at control changes only when that preserves behavior |
| `firmware/src/dsp/juno6.c/.h` | Six bounded voices, polyBLEP saw/pulse, sub, per-voice envelope/filter state, allocator, shared LFO, bend/damper | Compact Juno-style behavior is not full SuperJ6; two-pole filter, simple envelopes and 18 macros do not establish original semantics |
| `firmware/src/dsp/drum909_dsp.h` | 32-bit phase oscillator utilities, fixed-point sample position, block-reanchored exponential envelope, float biquads and existing table interpolation | Drum envelope/phase laws are useful primitives, not automatically the required synth laws |
| `firmware/src/dsp/master.c/.h` | Existing resonant stereo state-variable filter and float coefficient/control techniques | Master state itself is occupied; reuse code with separate voice histories and compare frequency/drive behavior |
| `firmware/src/dsp/busfx.h` | 68 B bounded filter/gate state; simple one-pole processing, shared timing utilities | It is bus FX and does not implement the original synth filter/modulation graph |
| `firmware/src/dsp/breaks.c/.h`, `ima_adpcm.h` | Existing bounded sample decoding, cursor/rate/interpolation techniques | BREAK voice/seek semantics differ from a synth sample oscillator; immutable PCM, loop endpoints, note rates and release policy need their own evidence |
| `juno6_chorus_process()` | 536 B independent chorus owner, including 256 × int16 samples and linear fractional read | One short mono history with opposite-sign stereo wet; not a verified original chorus topology |
| `app/chicago_cho.h` | Stock compact chorus/flanger; native 2,076 B owner | 1,024-sample int16 history, bounded modulation; original presets/control curves not assumed |
| `app/chicago_ens.h` | Three-tap compact ensemble; native 2,088 B owner | Three tap phases share one history by this effect's design, not across parts |
| `app/chicago_sdly.h` | Stock short delay; native 3,084 B owner | 1,536-sample history gives only ~34.83 ms at 44.1 kHz |
| `app/chicago_bpmdly.h` | Stock BPM delay; native 8,204 B owner | 4,096-sample history (~92.88 ms), with existing duration folding; not a faithful long-delay substitute |
| `app/chicago_hall.h` | Stock compact Schroeder network; native 4,548 B owner | The source's approximate size comment is misleading; it exceeds the 4,096 B selected-history arena |
| `app/chicago_gaterev.h` | Stock compact gated reverb; native 1,732 B owner | Gated character and tail behavior differ from an unrestricted original hall |
| `firmware/src/dsp/fxbus.c/.h` | Existing global send routing and resident compact delay/reverb | Shared global history is occupied and musically coupled; it cannot become a part-private effect merely by redirecting pointers |

`app/chicago_env.h` is a control/pot mapping helper, **not an envelope renderer**. The actual selected Chicago FX adapter uses a 4,096-byte arena for one chosen algorithm. It quantizes Hall allpasses to int16 and stores the BPM ring at half rate; these are existing product choices, not established acceptable substitutions for SuperJ6/Synth One.

Per the latest user constraint, the original SuperJ6/Synth One engine implementations and their derived envelope/filter adaptations are reference-only. They must not be copied or linked into the new candidate. In particular, the owner checkpoint's source-shaped compact envelope and `experiments/original_synth` reconstruction code are not approved new-engine building blocks by virtue of being present in the archive. Existing Chicago/Miami remain unchanged baseline dependencies for this resource measurement.

The current linked compact Juno function bodies occupy 2,920 bytes in total, including a 938-byte render body and 374-byte chorus body. This is named retained text, **not** an independently recoverable 2,920-byte savings or a full helper/adapter closure. A replacement must be linked into the complete image to determine its net effect; LTO, shared helpers, tables and registry changes make isolated object subtraction unreliable.

## Callback contract and existing fixtures

`firmware/src/seq/sequencer.h` fixes `FS = 44100`. `main_fm1.c` uses a 512-frame DMA half and two `engine_render(..., 256)` calls per half:

- Physical half duration: **512 / 44,100 = 11.609977324 ms**.
- One 256-frame render's equal-share duration: **5.804988662 ms**, not an independent hardware deadline.
- The firmware's integer diagnostic budget is `512 * 1000000 / 44100 = 11609` microseconds.
- `render_block()` includes render, USB tap, volume and USB host-output mix. Timer/radio/USB interrupt interference and multicore/XIP contention also matter on the target.
- The actual late/xrun path checks whether the DMA free-half changed while the IRQ rendered. Host elapsed time is not that counter.

For two fully active compact Juno parts, a 512-frame half has 6,144 bounded voice/sample iterations and 1,024 part chorus/sample iterations. Every voice/sample evaluates three polyBLEP edge functions, giving at most 18,432 edge evaluations for that baseline path. This bounds loop counts only; it does not convert into target cycles. Six-note note-on allocation scans and control-change coefficient work must also be included in burst cases. A new layered/two-oscillator engine changes these bounds.

Existing tests and fixtures:

| Fixture | What it covers / omits |
|---|---|
| `tests/host/audio_path_stress.py/.c` | Combined engine + volume, 512-frame workload, preset sweeps and optional every-block parameter/tempo automation; explicitly excludes target HAL/preemption; C/M configuration only |
| `tests/host/audio_stress_test.sh/.c` | Actual current-app functional/UI/control/drum/BREAK activity, with host transport substitutes; does not prove target DMA xruns |
| `tests/host/integration_perf_bench.c` | Selected C/M demand cases; not complete original-engine coverage or a proven worst factory |
| Recovered `DnBOX-integration-playback-qualification/fixture` | Tested nine ordered current-engine pairs, 180 simulated seconds each, raw timing/capture validation and deliberate negative controls; source `f753443c`, so its observations are historical input, not a current-76040 pass |
| `tests/target_budget.py` | Static loop-instruction heuristic, not cycles; its legacy function list omits the current Juno/Sound graph, so it cannot qualify a new synth deadline |
| `x0x_dbg`, `audio_xruns`, optional `x0x_worst`, `eng_prof` | Native timing/late/high-water instrumentation to retain for later authorized target qualification |

The recovered nine-setup playback report recorded **24 host CPU and 29 wall deadline misses** and did not run device qualification. It correctly leaves Synth One unqualified, since the common runtime is silent. Do not reuse its functional pass as all-engine timing acceptance or discard its maxima because a mean/p99 is low.

The archived S1 standalone benchmark's 128 frames at 48 kHz is a different contract. Any new integrated benchmark should use the actual 44.1 kHz application half while separately retaining component comparisons at reference rates. Once four real synth kinds are admitted into two slots, the ordered two-slot matrix has 16 combinations, alongside both drum kits, BREAK, global FX, control/automation/UI work. Engine kinds all compiled into the image need not all be selected in one two-slot instant.

## Concrete admission requirements for the Sound owner

1. Preserve current source/asset hashes and strict arithmetic settings. Build the new engine from allowed stock primitives and independent new behavior adapters. Keep original implementations exclusively in the comparison process.
2. Track all required simultaneous state, patches, immutable tables/PCM, FX histories, scratch and new registry/serialization data. The legacy `NPARAMS_MAX = 32` cell and Juno's 18 macros are not a complete 64-control binding.
3. Prove each part owns its histories and parameter state. Share immutable tables/code and prepared coefficients only when equal-input equivalence is demonstrated; do not share mutable envelope/filter/oscillator/FX history across voices or parts.
4. Link the complete candidate with both model builds. Preserve XIP free ≥1,024 B, pool raw free ≥8,704 B, ordinary RAM bounds, fully occupied XPOOL constraints, stack guards and the no-software-double gate. The measured stock baseline is only the comparison input.
5. Measure helper closure and stack use in the integrated candidate. Native disassembly contains unknown FPU decoder entries, so this review does not claim a complete static call-graph/stack/cycle proof from it. Existing stack capacities are known; fresh maximum usage is not.
6. Exercise worst control/cache-change bursts, allocator/steal/release cases, sustained maximum voices/layers, simultaneous FX and long tails, both slot orientations, cold changes, queue pressure and the whole callback. Retain maxima and missed-deadline counts separately from averages.
7. Keep component behavior, host functional acceptance, host timing and native/device acceptance separate. No new engine, full fidelity, all presets, complete Local gate or hardware timing is asserted by this review.

## Files to consume

- `native-baseline-76040/`: exact compiled baseline source and local native outputs; no production repository mutation.
- `native-baseline-source-manifest.json`: 379 source/input files with current Git blob, byte count and SHA-256.
- `native-baseline-elite-command.json`, `native-baseline-pro-command.json`: exact commands and paths.
- `native-baseline-elite.log`, `native-baseline-pro.log`: successful stock model builds; missing-input attempts remain separately saved.
- `native-linked-ledger.json`, `native-*-sections.txt`, `native-*-symbols.txt`: linked region and owner evidence.
- `resource_layout.c`, `resource_layout-command.json`, `resource_layout.o`, `resource_layout.bin`, `resource_layout.json`: native ABI measurements.
- `native-environment.json`, `native-build-results.json`: environment and execution receipts.

Primary source links: [current engine](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/app/engine.c), [linker map definition](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/app.ld), [build checks](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/tools/build.py), [current runtime](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/app/original_runtime.c), [current compact Juno](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/dsp/juno6.c).
