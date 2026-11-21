# DnBOX hardware candidate: analytic core and evidence

This is an isolated, reusable DSP candidate, not a completed Sound engine or a production integration. One caller-owned part supports six allocated notes; each note contains two independent oscillator/filter layers and separate amplitude and filter envelopes. Two part instances have no shared mutable state. The implementation uses the permitted stock DnBOX oscillator/filter/allocator primitives and an independently derived curved envelope fitted to the permitted numeric amplitude observations.

## Scope and provenance

The only implementation/reference inputs read were:

- Stock `juno6.c`, `juno6.h`, and `fastmath.h` in the specified `76040daa5b52d686a8b7b544296ba5d79610a95d` tree.
- The four files in `mapping/results/implementer_brief`: public control identities, synthetic scalar targets, recorded numeric behavior metadata, and inferred amplitude levels.

The specified stock tree did not contain `breaks.c` or `breaks.h`; no alternate copies were read. No original engine source, prior source adaptation, original sample asset, source-aware implementation report, or production source was used. `src/fastmath.h` is an unchanged copy of the permitted stock file. Exact input and candidate hashes are in `evidence/source_provenance.json`. All code and test artifacts remain in this isolated directory; no Git, production, device, PSRAM, external Codex, or Actions work was performed.

The oscillator/filter primitives retain the GPL-3.0-only attribution of the permitted DnBOX sources. The original recordings remain behavior references; they are not shipped oscillator tables or sample assets.

## Deliverable and build

| File | Purpose |
| --- | --- |
| `src/dnbx_core.h` | Complete C ABI, fixed caller-owned layouts, parameter units, and status codes |
| `src/dnbx_core.c` | Candidate core; no heap, I/O, locks, or mutable globals |
| `src/fastmath.h` | Unchanged permitted DnBOX single-precision math dependency |
| `tests/test_core.c` | Functional, lifecycle, isolation, bounds, and partition tests; trace exporter |
| `tests/bench_core.c` | Two-part host wall/thread-CPU timing with explicit stimulus |
| `tests/assess_evidence.py` | Numeric fit/agreement, source provenance, symbol audit, and size ledger |
| `evidence/` | Executed results, trace CSV, qualification manifest, and hashes |

From this directory:

```sh
make all
make test
make sanitize
python3 tests/assess_evidence.py
./build/bench_core > evidence/host_callback_timing.json
```

After relocation under a repository's `experiments` directory, the assessment helper can discover the owning `firmware/src/dsp` or accept `--stock-dsp PATH` / `DNBX_STOCK_DSP`. It verifies the exact permitted SHA-256 of all three stock files and fails on a mismatch; it does not accept newer/different stock sources silently. `--verify-inputs-only` verifies relocation without writing receipts. `--output-dir PATH` writes a fresh receipt set separately. The original numeric observations and historical receipts are not rewritten merely to change their recorded paths.

The strict host build uses C11, `-O2`, `-ffp-contract=off`, `-fno-fast-math`, `-DX0X_HOST`, and warnings as errors. The core does not require libm. The test executable uses normal host test support. Sanitizer execution disables LeakSanitizer because this container reports its ptrace incompatibility; AddressSanitizer and UndefinedBehaviorSanitizer remain enabled. The compiled core's undefined-symbol audit contains no allocator or I/O dependency. This does not imply that caller or full-product code is allocation free.

## Executable ABI and ownership

ABI version is `0x00010000`. `dnbx_part`, `dnbx_layer_params`, and `dnbx_work` have public layouts for static allocation; the implementation state fields must not be modified by callers. Size-query functions are available for foreign-function harnesses. All inputs and outputs use binary32-sized `float`; note/control IDs use the documented integer types. Calls on an instance must be serialized. Different instances may run independently.

The normal sequence is:

1. `dnbx_init(part, sample_rate)` with 8,000–192,000 Hz inclusive.
2. Populate physical layer parameters with `dnbx_default_layer_params`, then call `dnbx_set_layer` for each layer.
3. Select `dnbx_set_velocity_mode` and submit note/pedal events at their exact sample boundary.
4. Call `dnbx_render(part, left, right, frames, work_or_null)` for 0–1,024 frames. Events take effect before the next sample.

`dnbx_render` overwrites the supplied stereo arrays. The arrays must each have enough capacity and must be disjoint from each other, the part, and the optional counters. **Buffer capacity and partial/state/work nonoverlap are caller preconditions, not runtime validation.** The implementation checks null pointers and exactly equal L/R pointers; it does not detect arbitrary aliasing. The header's nonmutation promise concerns the checked argument failures. A zero-frame call accepts null output pointers and advances nothing. There is no concurrent UI/audio publication protocol in this candidate.

Return code `0` is success for ordinary functions; negative values identify a checked error. Positive-velocity note-on returns its selected voice index 0–5. Velocity zero performs note-off and returns `DNBX_NOTE_OFF_ONLY` (6). MIDI note and velocity must be 0–127. Pedal state and velocity mode are strict enumerations. Invalid physical parameters are rejected atomically; they are not silently clamped into a valid patch.

`dnbx_set_layer` immediately updates oscillator/filter/LFO controls and active pitch increments. A running timed envelope segment keeps its cached endpoint and rate; subsequent segments use the current configuration. A currently sustained envelope adopts the new sustain level. This explicitly defined policy is not claimed to match original live-automation semantics.

## Physical controls and explicit modes

| Control | Units/range and behavior |
| --- | --- |
| Amp and filter envelope stages | Independent AHDSHR structures: attack, hold, decay, release hold, release each 0–16 seconds; sustain 0–1 |
| Waveform | Stops 0 sine, 1/3 triangle, 2/3 saw, 1 square; linear interpolation between adjacent stops |
| Oscillator mode | `DNBX_OSC_STOCK_BLEP` or `DNBX_OSC_RAW`, independently per layer |
| Pulse width | Physical duty fraction 0.03–0.5 |
| LFO rate | Physical 0–16 Hz; zero freezes that layer's phase |
| LFO PWM depth | Duty excursion 0–0.47; final duty is bounded to 0.03–0.5 |
| Layer gain | Explicit linear 0–4; 1.32 is accepted exactly. This is not an inferred app volume law |
| Sub gain | Linear 0–0.5 for the stock half-frequency square primitive |
| Cutoff | 20–20,000 Hz, converted through the bounded stock two-pole coefficient |
| Filter envelope excursion | Signed −20,000 to +20,000 Hz |
| Resonance | Stock normalized 0–1 feedback parameter, multiplier 0.8 |
| Tune | −48 to +48 semitones; oscillator increments bound to 0.000001–0.45 cycles/sample |
| Pan | −1 left, 0 equal half gains, +1 right; linear gains |
| Velocity mode | Linear `v/127` or squared `(v/127)^2`, explicitly selected per part; changing mode also affects existing notes |

Raw mode removes the stock BLEP terms from saw and pulse. It does not claim that the stock BLEP algorithm matches an original enabled-bandlimit path. The triangle is the analytic stock-style triangle, not a separately qualified original bandlimited triangle. The square suboscillator remains the stock raw half-frequency primitive. Oscillator phases and blend alignment are defined by this implementation and have not been qualified against a complete original instrument.

The filter is explicitly the stock approximate two-pole recurrence, with finite saturation guards at ±4 in each state. It is not an accepted replacement for the different Synth One reference transfer. Separate linear/squared velocity and raw/BLEP modes preserve the known behavioral choices without declaring either entire engine accepted.

The standalone scalar helpers implement the supplied known relationships: attack `0.002 + 15.998*u^3.33`, LFO Hz `16*u^3.33`, PWM duty `0.5 - 0.47*u`, and exact range stops 0/0.5/1 → 0.5/1/2 frequency ratio. These helpers do not bind an app ID to a layer or infer undocumented selectors, thresholds, modulation routing, or other control transfers.

## Raw values: retention is distinct from DSP implementation

The part retains all 218 finite raw scalar values plus presence bits. `dnbx_store_raw` stores exact float values, including values outside 0–1. No raw ID automatically changes DSP. Returned status distinguishes ordinary retained data, reserved ID 207, and externally owned key-lock metadata IDs 212–214. Storing that metadata does not modify an external/global key-lock state.

The admission tests retain gain 1.32, route selector 70, pad selector 9, and sequence value −0.008403362. NaN, infinity, and out-of-range IDs are rejected. `evidence/raw_control_coverage.csv` explicitly marks **all 218 app IDs as having no automatic DSP binding**. The physical prototype supports the controls described above; it does not represent 218 implemented app controls.

## Envelope derivation and comparison

For a segment starting at `s`, ending at `e`, and approaching an asymptote `a` beyond `e`, the generic curve is:

```text
y(t) = a + (s - a) * ((e - a) / (s - a))^(t / duration)
```

Least-squares fits to consecutive observed endpoints estimate an attack asymptote near 1.2231306 and falling-segment asymptotes near endpoint −0.00708345. Those two rounded constants define this candidate. The implementation computes the step at stage entry with stock float math, then performs bounded tick updates and per-sample interpolation. Small exponent steps use a stable polynomial for `1-exp(x)`; compensated accumulation prevents early endpoint drift during a 16-second attack.

The numeric observations show that the first 16 attack frames are flat at the first endpoint, while later blocks interpolate. A completed positive-duration attack with zero hold spends one internal 16-frame hold tick. The candidate reproduces those observed scheduling rules independently of host callback boundaries. Zero attack and consecutive zero stages settle immediately; they do not inherit that minimum hold. A one-sample positive attack still receives the post-attack hold tick. This distinction is explicit and regression tested, including zero-attack retrigger exactly one sample after an earlier nonzero attack.

At the original exact first/last sample timestamps, without temporal shifting:

| Numeric capture | Compared endpoints | Maximum absolute amplitude error | RMS amplitude error |
| --- | ---: | ---: | ---: |
| Case 1, 10 ms attack / zero hold / 50 ms decay / 100 ms release | 1,024 | 6.94 × 10⁻⁷ | 2.12 × 10⁻⁷ |
| Case 2, 30 ms attack / 20 ms hold / 200 ms decay / 40 ms release hold / 300 ms release | 4,096 | 7.95 × 10⁻⁷ | 2.00 × 10⁻⁷ |

These are the same calibration observations used to derive the generic constants, not held-out proof. They describe the inferred multiplicative amplitude path of the recorded selected component. They do not establish full original audio, pitch/filter modulation, app-wide velocity behavior, active original filter-envelope curves, or original 44.1 kHz alignment. The root's separate acceptance worker owns independent negative controls and final acceptance.

## Lifecycle, allocation, and regression evidence

Allocation follows the stock policy classes: same held note, first fully idle slot, first released slot, then oldest held note. Same-note retrigger preserves oscillator/filter state and starts attack from the current envelope level. A new or stolen note resets that voice's oscillator/filter/envelope state. Oldest ordering uses compact bounded ranks, not a wrapping timestamp. Rank compaction uses at most 36 comparisons per note-on, outside render.

Pedal-held notes release on pedal-up. Already-releasing notes are not restarted when a pedal is pressed and released later. Note-off during interpolation starts from the last produced envelope value. `dnbx_all_off` clears all voice state, pedal state, and both LFO phases while retaining parameters/raw values/velocity mode.

The functional run reports **12 tests, 465,568 assertions passed**, including trace export. ASan/UBSan reports **12 tests, 424,596 assertions passed** without trace export. Tests cover checked invalid inputs, raw preservation, scalar/waveform laws, raw/BLEP difference, zero and one-sample restart, independent amp/filter timing, a 16-second attack, note stealing, retirement ordering, retrigger, release, sustain, two-layer independence, two-part isolation, both velocity laws, bit-identical irregular block partitioning, legal extreme parameters at five sample rates, finite output/state bounds, and counter consistency.

The independent reviewer found rank ties after repeated retirement/reuse. Its exact sequence was added to worker tests, failed on the preceding source, and passed after compaction. `evidence/regression_history.json` records that correction and the earlier long-attack precision correction. Source freeze hashes are recorded by the provenance file; prior failed snapshots are not silently presented as final.

## Resource bounds and timing limits

| Caller-owned storage | Bytes |
| --- | ---: |
| One part, including raw-value retention | 2,264 |
| Two complete parts | 4,528 |
| One voice, both layers | 192 |
| One envelope | 36 |
| Layer parameter structure | 96 |
| Optional external semantic counters | 112 |
| Raw values plus presence bits inside each part | 900 |
| One candidate part plus existing 536-byte stock chorus | 2,800 |
| Spare within existing 3,720-byte replacement backend slot | 920 |
| Measured native adapter, including its surrounding state | 2,356 |
| Measured native adapter plus unchanged stock chorus | 2,892 |
| Actual adapter/chorus spare within that backend slot | 828 |

This is **replacement state**, not a proposed extra `.pool` allocation. Existing selected FX are 4,252 bytes per part and are outside this candidate's redesign scope. The owner-supplied current `.pool` is 335,092/344,064 bytes: raw free 8,972, reserve 8,192, margin 780, only 268 beyond the required additional 512. Existing XIP free space is 1,180/1,188 bytes against a 1,024-byte floor, leaving only 156/164 bytes before stock-code replacement. A small state footprint alone cannot prove product fit. The native resource worker owns actual replacement links, normal section garbage collection, strict size optimization, and the final flash verdict.

Render loops are bounded by requested frames (at most 1,024), six voices, two layers, two envelopes, and at most six immediate stage transitions per envelope evaluation. Each active layer evaluates at most two adjacent shapes; the saw/square blend uses at most three BLEP calls per layer sample. There are no heap operations, delay buffers, per-render allocations, locks, or elapsed-time retry loops. The explicit filter/amp/gain limits bound each output channel to magnitude 32 for valid state and nonaliasing caller buffers. This is not protection against callers directly corrupting public state.

`dnbx_work` reports semantic render work only. It does **not** count note-on/off, control application, initialization, allocation rank scans, or math performed by those calls. With six active voices for 128 frames, one part reports 768 voice slots, 1,536 layer samples, 3,072 envelope samples, 256 LFO samples, and 4,608 BLEP calls for the saw/square blend. These are not CPU cycles or native instruction counts. Counters accumulate into caller-owned storage and are optional.

The benchmark uses two parts × six voices × two layers at 44.1 kHz, a 512-frame callback composed of two 256-frame subblocks, and 2,000 measured callbacks per case. It records calling-thread CPU and wall-clock maxima with their callback indices, plus means and one instrumented callback's render counters. Stimuli cover stock BLEP, explicit raw oscillators, long moving curves, and a complete timed burst containing all-off, four full layer-control applications, twelve note-ons, stage transitions, and the same audio work. Timer-call overhead remains included. Semantic counters in the burst still exclude the events/controls; the burst timing includes them.

`evidence/host_callback_timing.json` is the measured host receipt when present. The independent test-only trace harness in `../candidate_review` supplies the separate selected event/control work audit. Both harnesses measure an engine/event slice, **not the full firmware callback** with global FX, drums, BREAK, mixing, USB, and other owners' work. Host timing does not establish target hardware deadline safety; full target WCET remains unmeasured. No device timing, listener approval, full FX graph qualification, all-engine coexistence proof, or whole-product acceptance is claimed.

### Executed host observations

The controlled worker run began after the native resource worker reported all compiler/linker jobs terminal. Its 512-frame / 44.1 kHz slice represents 11,609.977 microseconds of audio. No outliers were discarded:

| Two-part stimulus, 2,000 measured calls each | Maximum wall time (µs) | Maximum calling-thread CPU (µs) | Mean calling-thread CPU (µs) |
| --- | ---: | ---: | ---: |
| Steady stock BLEP, all voices/layers | 1,998.739 | 1,997.522 | 310.597 |
| Steady explicit raw oscillator mode | 2,486.405 | 2,484.113 | 251.777 |
| Long moving curves, stock BLEP | 2,192.741 | 880.435 | 258.114 |
| Full selected control/note/transition burst plus rendering | 1,516.520 | 1,515.056 | 266.229 |

These maxima belong to this host run and test stimulus. The lower average of a case does not imply a lower worst-case observation, and none of these rows establishes target timing.

### Native replacement result and remaining flash blocker

The native worker measured actual replacement call graphs preserving the other engines and existing FX, with normal section collection and strict `-Oz`. It also tested the separate bounds-table / explicit diagnostic-profile experiment in `size_variants`. This is not an inference from simply adding a second engine to the image.

| Native reachable replacement | Elite image bytes | Pro image bytes | Elite shortfall from required free-XIP floor | Pro shortfall |
| --- | ---: | ---: | ---: | ---: |
| Frozen functional core | 553,572 | 553,564 | 1,924 B | 1,916 B |
| Separate compact table/profile variant | 553,420 | 553,412 | 1,772 B | 1,764 B |

The compact variant saves 152 image bytes. Enabling versus disabling counters gives the same native reachable image size because LTO already specializes the adapter's null-work calls. The smaller image still physically exceeds capacity by 748/740 bytes and misses the required reserve by 1,772/1,764 bytes. **Native flash fit remains blocked.** The 2,892-byte adapter/chorus state fits the existing replacement backend; that state result does not waive the flash blocker.

`evidence/native_resource_summary.json` records the native worker's exact receipt paths and figures. The native resource package owns the compiler commands, timestamped links, symbol/stack analysis, and full receipts. The frozen core and the separately qualified compact variant remain distinguishable in provenance; the experiment has not silently replaced the baseline deliverable.
