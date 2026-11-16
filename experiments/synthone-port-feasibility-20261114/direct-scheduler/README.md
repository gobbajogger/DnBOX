# Synth One direct-event scheduler source prototype

This cut adds the upstream direct-note event and block scheduling behaviour to the genuine six-note pre-FX graph. It keeps six fixed poly states and a separate seventh mono state, one shared PRNG/LFO context, borrowed immutable waveform/tuning data and caller-owned DSP/control state. It changes no firmware source or shared graph/control module.

The reference is AudioKit Synth One `6466a3715c96b7ecf1dd255a218cbd571408a314` with AudioKit `03fecf80a66be88f6ac0356cd532c72dbe09922d`. Existing graph input is project #934 `a0c8698f724e28ec6275307525933c195817b3c5`. `DEPENDENCIES.json` pins every compiled external dependency, the shared graph header, source metadata and the already prepared public waveform payload. The input tables retain their original float32 bits and overshoots. No supplied presets or rendered audio are included. `init` represents constructing a fresh logical instance; it does not promise to reproduce the upstream `init()` method called on an already live kernel.

## Supported boundary

Calls are serialized. Arp must be exactly zero. The caller establishes the original `sequencer.reset(true)` empty-pending-list precondition; `init` and the subset `restore` explicitly perform that precondition step after parameter restoration. `EmptySequencer` asserts that domain at each frame. This adapter does not implement the original initializer's 1024 pending zero notes, arpeggiator or sequencer.

`Scheduler::start` reproduces kernel `startNote`, including ignored explicit frequency, pitch-deduplicated held recency, current transpose and tuning lookup. `on` reproduces `turnOnKey`: its explicit-frequency overload uses that frequency, while its two-argument overload resolves current transpose/tuning. `stop` edits the held array before `turnOffKey`; `off` acts directly on voices. The fixed array preserves head-first iteration order and supports all 128 distinct pitches. It models no channel, key owner or press count.

The input domain is pitch/velocity 0 through 127, finite explicit frequency from zero through Nyquist, and finite resolved parameter inputs (clamped to the existing pinned physical ranges). Arp-on is rejected. Tuning supplies 128 finite positive frequencies no higher than Nyquist; the source lookup clamps the final transposed index. Supported sample rates are 44.1, 48 and 96 kHz. The borrowed waveform and tuning spans must remain readable and immutable throughout the instance lifetime. The actual source rate snapping, dependent-parameter routing, tuning generation and AU wrappers remain outside this API.

Invalid admitted input is rejected before scheduler state changes. Output spans must be distinct and must not overlap the instance, tuning or any table payload. `process` accepts at most 65,536 frames including offset; the caller supplies enough writable output memory. Input pointers must point to readable spans of the stated size. `init` rejects parameter input that overlaps its object; `restore` permits exact aliasing with its parameter outputs and rejects partial object overlap. Tables and tuning cannot alias instance storage. The API does not provide concurrent access or arbitrary pointer validation.

## Preserved source behaviour

- Poly allocation first finds the same root pitch, then a free root sentinel from the current cursor, then advances that cursor once to steal. Cursor survives musical resets. Retrigger/steal retain per-slot oscillator, filter and envelope history.
- `process` clears release voices below 0.01 only at block entry and only in the selected mono/poly bank. A voice crossing below the threshold mid-block stays eligible through that block. Equal-to-threshold remains occupied.
- Each frame advances compact parameter outputs, mono-frequency smoothing, raw `isMono` change reset, shared LFOs, the empty-sequencer precondition and eligible notes in ascending poly-slot order. Sparse holes never consume noise draws. Eligible notes still consume shared PRNG noise when noise volume is zero.
- Poly note-off performs the original extra two ADSR computes immediately. Zero `monoIsLegato` forces that release step on mono retrigger/fallback; nonzero does not. Mono fallback also occurs for non-head or absent valid note-offs. Its omission of the noise-amplitude update is retained.
- Fractional positive `isMono` changes reset both note banks even when both old/new values are positive. Mono smoothing runs before that reset. Current transpose/tuning apply to fallback.
- Note `clear` changes only the original metadata/gate/amp fields. Musical `reset`, `reset_dsp`, release-based `stop_all`, fresh initialization and parameter/note restoration remain separate operations. `restore` resets parameter smoothing accumulators and marks note DSP for lazy initialization while retaining source note metadata not assigned by `initializeNoteStates`.

The static initializer is checked against actual unchanged `S1NoteState::init` and its source setup field assignments. Test heap allocations are zero-filled solely to make unused initial allocation bytes deterministic, following the existing graph oracle convention. Production objects are zero-initialized.

## Reference and checks

`oracle/generate.py` verifies exact upstream byte lengths, Git blob IDs and SHA-256 before extracting eleven actual method bodies or process-prefix bodies. Its manifest records original/adapted body hashes and every exact-count Objective-C/container/output adaptation. The source process retains the full original parameter-port loop, mono port and LFO arithmetic independently from the prototype's compact port tick. The runner regenerates and compares the oracle before compiling.

Focused tests compare exact pre-FX PCM buffers, guards and canonical field bytes at block entry, after every frame and after direct events. The snapshots include every declared field of all note DSP objects, all selected port states/targets, mono/LFO state, held lists, cursor, PRNG/context and table descriptor metadata. Pointer ownership and immutable payload identity are explicit; object addresses/padding are excluded. Separate original `S1NoteState::init` calls check fresh and lazy-restored static note initialization. Event/render actions trap malloc/calloc/realloc/free. Tables and tuning are mapped read-only.

Run the changed cut with explicit input paths:

```sh
python3 run_tests.py --experiment-root /path/to/verified/experiment \
  --controls-root /path/to/verified/control-module \
  --master-root /path/to/pinned/master-source \
  --tables /path/to/verified/public-morphs.f32 --variant o2
```

### Portable control-package binding

The public consumer pins the actual four compiler files under the sibling `parameter-ports/` package. Its public `parameter_ports.hpp` is 1,836 bytes with SHA-256 `b533cc60b93da7df64aca34be3f382ff0191ffd2c4f6db6275bffcd06c665f61`; public `generated/parameter_map.hpp` is 445 bytes with SHA-256 `c7bc2f49818d4f0c202b6b1c79ea7552f8a70476ff388889636908f7421ca11e`. Their only differences from the original producer inputs are documentation comments. Both C++ implementation files and all declarations/arithmetic remain unchanged. The staging step copies these authenticated public bytes without rewriting includes or comments.

Use `--smoke` to compile this exact portable consumer boundary and execute only three short direct-event/control cases, one per supported sample rate. Its generated entry point reuses the current fixture's typed canonical state/PCM and no-heap helpers; the full 54-case entry point is renamed and never called by the smoke. This checks portable package consumption, not the full scheduler fixture. Historical 48-case evidence remains under its original source/input freeze. The default full-fixture mode executes the current 54 cases, with fresh successor evidence recorded below and in `STATE-SPLIT-RESULT.json`.

Use `os` or `sanitize` for the other host variants. Inputs are hashed and their verified bytes copied into a fresh compiler closure before compilation. Unlisted caller-tree headers cannot shadow the pinned headers. Ambient `CPATH`, `C_INCLUDE_PATH`, `CPLUS_INCLUDE_PATH` and `OBJC_INCLUDE_PATH` are cleared for compiler/linker calls. The exact local code is also staged and recorded in `COMPILED-SOURCES.json`; each run uses a fresh object directory and an explicit link closure. An optional `--build-root` keeps a review run in its own directory. Unchanged upstream C/DSP warnings remain visible and are not made fatal. Prototype, adapter, control and test translation units use strict warning checks.

`mutation_checks.py --build /path/to/completed/o2-run --tables /path/to/verified/public-morphs.f32` compiles synthetic mistakes against that run's exact objects. It requires each mutation to compile/link and then fail the runtime assertions, so compiler errors do not count as detected behaviour regressions.

## Limits and next stage

This is a pre-FX direct-event subset. All four global-delay reset calls in the reference are explicit no-ops in this oracle. Reset results qualify retained note/shared/control state only; they say nothing about delay, reverb, phaser, compressor or other global tails. No full-engine acoustic equivalence, full AU/event slicing, MIDI normalization, sustain wrapper, AEArray thread timing, arp sequencing, device or integrated CPU/resource acceptance is claimed.

The original #939 common state carried reference-only parameter-port storage. This successor separates that scaffolding into `OracleState`; the runtime keeps only compact ports. Neither host layout is accepted target memory layout. Before production integration, combine the scheduler with the real graph/control/storage ownership, qualify the target table-read path, and measure exact combined native flash/RAM and worst-case workload. Preset interpretation and all global FX still need their own genuine source stages. The firmware candidate remains unchanged by this experiment.

MIT notices are retained in `notices/` and the pinned dependencies.


## Runtime/oracle state split successor

This bounded successor separates the original parameter-port arrays from the runtime instance. `State` now owns only the common graph/lifecycle; `RuntimeState` owns compact ports, and the handwritten `oracle/state.hpp` owns original `S1Info[150]`, `sp_port[150]` and the pre-FX null delay tokens. The runtime event/render arithmetic and generated `oracle/oracle.cpp` remain byte-identical to #939. This removes oracle-only runtime storage, without trimming note banks, held lists, graph/context or source histories.

Final `Scheduler` and `Oracle` wrappers supply the complete concrete object span for input/output alias checks. A common-base size is not substituted. Both concrete types explicitly reject copy/move. `restore(parameters,150)` retains its exact-alias exception and snapshots values before replacing controls; partial, compact-port and original-port storage aliases reject before mutation or dereference. The table wrapper itself also cannot overlap the instance. Waveforms/tuning remain borrowed immutable payloads, not owned allocations.

The original flags are now extracted independently from the authenticated upstream `S1DSPKernel.hpp`, instead of constructing the oracle flags from the compact map. Tests require all 150 flags and the 46 selected indices to agree before comparing typed runtime/oracle snapshots. The oracle still advances its own original port loop. Its handwritten restore copies outputs directly and invokes actual `sp_port_init`, retaining c1/c2 and resetting history/dirty state independently from compact restore. Parameter 144 still updates half-time from the clamped current output. Typed trace callbacks cannot confuse sibling state through a Boolean-based cast. Original oracle event/process bodies remain trusted test-reference entry points with their existing caller preconditions; `Scheduler` is the validated runtime API.

Fresh changed-closure GCC O2 and Os runs each pass 54 cases, 18,162 frames, 19,302 canonical state snapshots and 648 events, with zero intercepted heap calls during actions. Six added rate-specific cases cover concrete derived-storage alias rejection without state/output changes, exact output restoration, independent live instances, address-stable internal DSP bindings and live reinitialization against fresh state. All ten mutations compile/link and fail at runtime. The relocated oracle half-time mutation rebuilds every local translation unit against its isolated changed adapter, not stale inline definitions.

The default sanitizer runner compiles successfully but exits 1 because LeakSanitizer reports that it cannot operate under ptrace in this executor. The exact resulting binary, rerun with `ASAN_OPTIONS=detect_leaks=0`, passes the same full 54-case fixture with AddressSanitizer, UndefinedBehaviorSanitizer and float-cast-overflow enabled. This is not a LeakSanitizer pass. The baseline 48-case fixture and pre-change failed storage-separation assertion are retained separately. Existing upstream `ftbl.c` signedness warnings remain; changed C++ builds use `-Werror`.

Measured host `sizeof` on this compiler/ABI (8-byte pointers, 4-byte float): common State 11,064 bytes, Scheduler/RuntimeState 11,480 bytes, Oracle/OracleState 18,912 bytes. Baseline Scheduler was 19,312 bytes; the measured runtime object difference is 7,832 bytes, including moved delay tokens. This is a host layout result, not target RAM savings, external payload cost or native acceptance. Object addresses/padding are not canonical serialized state.

`STATE-SPLIT-RESULT.json` and `STATE-SPLIT-REVIEW.md` describe only this successor. Original `RESULT.json` and `REVIEW.md` remain unchanged historical baseline proof. Unchanged graph, portable-control and bank suites are reused under their recorded input pins, not replayed. No firmware, SDK, supplied bank, audio, native resources/CPU, complete-engine, device, merge or release acceptance is claimed.


## Freestanding target-portability successor

The #940 source/host acceptance above did not establish that its C++ standard-library headers were available on the embedded compiler. A real pi32v2 preflight compiled fourteen DSP/voice units but rejected Scheduler, Ports and its structural map at missing `cstddef`. This successor uses ordinary C headers and classification/limit names, plus a minimal owned `FixedArray` implementing only indexed/range access needed by the scheduler. It introduces no fake standard library or heap container and rewrites no DSP/event arithmetic. Parameter-port formulas/maps, all original oracle bodies and current source histories stay unchanged. The control supplier's strict input identities and the scheduler consumer's dependency identities are refreshed together.

Fresh changed-input O2 and Os scheduler fixtures each pass 54 cases, 18,162 frames, 19,302 canonical snapshots and 648 events. The same fresh ASan/UBSan/float-cast-overflow fixture passes with leak detection disabled; the default run still fails because LeakSanitizer cannot operate under ptrace here. Ten scheduler mutations compile/link and fail runtime assertions. The changed control supplier's full O2 proof passes 1,073,850 output comparisons and 329,314 selected seven-field snapshots; its 51 input negatives and eleven compiled runtime mutations pass detection. These are host/source results, not target execution or acoustic acceptance.

`run_native.py` requires explicit public dependency roots and the existing Linux JieLi toolchain root. It pins the compiler and every used toolchain header through `NATIVE-TOOLCHAIN.json`, stages only authenticated public runtime inputs, uses a minimal fixed compiler child environment rather than inheriting ambient driver/loader/search controls, disables default include directories, and records full native dependency/object identities. It produces seventeen runtime objects and a separate compiler-evaluated layout object; it does not link firmware, embed table payloads or run target DSP.

```sh
python3 run_native.py --experiment-root /path/to/verified/experiment \
  --controls-root /path/to/verified/control-module \
  --toolchain-root /path/to/jieli-linux-toolchains-20260730.1 \
  --build-root /path/to/isolated/native-output
```

With this exact pi32v2 r3/mfprev1 profile, the compiler reports 4-byte pointers, 4-byte float, 8-byte double, Scheduler/RuntimeState 9,956 bytes, common State 9,552 bytes, compact Ports 396 bytes, seven note DSP-object banks 3,108 bytes and 53 table descriptors 1,484 bytes. These are genuine target-ABI type sizes, not free pool, linked app size, stack high-water or fit. Host layout remains separate. Exact link, current Juno co-residency/resources, bounded immutable table-read placement, workload/CPU, full-engine/global-FX, firmware/device and release qualification remain open. Native objects, toolchain/SDK bytes and generated tables are not public artifacts.

The maintained policy/nightly pin is not automatically a current Juno integration source. Any combined firmware budget must name its actual non-denied source closure and retain its integration owner. The separate exact-row aliases in #936 still need a scheduler composition proof and an actual bounded lossless target-read representation; raw float storage must not be assumed to fit app flash or existing user/sample regions.


Independent portability review found that inherited `CCC_OVERRIDE_OPTIONS` could enable finite-math assumptions despite the recorded strict argv. The repaired native runner inherits no ambient variables and supplies only fixed system PATH/locale plus its owned temporary directory. Its regression first failed on changed executable sections, then required all seventeen runtime objects' executable sections to match across clean and poisoned parent environments. This is an object-level reproducibility check, not target execution or acoustic proof.


## Exact immutable aliases composed with the direct scheduler

The #941 stage above established native objects, not table/storage composition. This successor adds only host test/runner evidence: the runtime now consumes a physically separate read-only50-row packed binding with exactly logical48→44 and49→45, while the unchanged original oracle retains its original52 distinct rows. The original DSP, runtime, oracle/control formulas and native runner are byte-identical to #941. No generated tables or firmware binaries are delivered.

```sh
python3 run_tests.py --experiment-root /path/to/verified/experiment \
  --controls-root /path/to/verified/control-module \
  --master-root /path/to/pinned/AudioKitSynthOne \
  --tables /path/to/verified/public-morphs.f32 \
  --alias-composition --variant o2 --build-root /path/to/isolated/results
```

The fixture compares every213,044 delivered logical word, validates all52 borrowed bindings separately, and exercises7,371 exact oscillator cases over all13 bands, morph-shape boundaries, block edges and phase endpoints across all3 supported rates. It also compares1,701 positive scheduler PCM frames and2,571 canonical state snapshots through asymmetric six-note high-band activity, mono/poly changes, release, reset, restore, reinit and independent instances. Seven attempted writes to the actual protected payload/sine addresses terminate their child processes; three deliberately wrong nonduplicate aliases produce a real oscillator-output difference. Three separate scheduler-path wrong-row48 controls render1,536 frames and demonstrate finite, audible PCM divergence. The correctly bound positive path explicitly enables bandlimiting, selects alias-region morph shapes and schedules pitches122–127. A test-only linker wrapper observes source-defined selections at real runtime morph-call arguments:10,308 calls,2,304 row48 selections and4,608 row49 selections, with per-rate positive assertions. The highest active voice is explicitly retriggered then released; both runtime and oracle must report its retained pitch and stageRelease. All189 observed first-morph calls during those three release blocks select alias rows. An independently compiled inactive-off control fails the release-stage assertion after linking successfully. These are software selection deductions, not hardware-load traces; the wrong-row PCM controls independently prove data consumption. Real no-note Scheduler/Oracle processing warms cold control ports for376,200 combined frames with block PCM/state equality. No output or port history is manually poked to force a selection. No heap operation occurs during measured DSP/control actions.

Physical table addresses are intentionally different, so this new fixture serializes the logical table index instead of its raw address. All other descriptor and canonical state fields stay exact. Direct binding assertions and exhaustive bit checks are independent of that normalization. The existing54-case fixture still serializes raw pointers by default and passes unchanged metrics. The new fixture first failed when the runtime mistakenly retained the distinct-row binding, then passed after that test binding was corrected. Independent review later found a P2: source defaults disable bandlimiting, so the initial scheduler renders never selected duplicate rows. Its coverage assertions fail before repair. The repaired fixture requires real row48/49 selection in each rate and full-scheduler wrong-alias PCM divergence. Failed partial repairs are retained; merely enabling bandlimiting/high pitches is insufficient while the original cold detuning port collapses frequency. The actual no-note warmup and controlled morph positions address that source behavior without changing it. A second review P2 found that the old off(121) no longer targeted an active pitch after the note change. The final active-voice release assertions and scheduled alias-release observation close that gap; the failed inactive-off control is retained.

O2 and Os pass. The exact sanitizer binary passes with ASan/UBSan/float-cast-overflow and leak detection disabled, stderr empty. Its default LeakSanitizer run fails under ptrace and is not a leak pass. Separate initial missing-dependency-root setup failure is retained and is not counted as the behavioral RED.

This closes only the source/host alias-to-scheduler boundary. Sine is shared with the unchanged host fixture and is not target-libm regeneration proof. The50 raw unique morph rows remain819,400 bytes, plus the separate16,388-byte sine, and do not fit the entire existing app-code region, let alone the remaining firmware space. No bounded native decoder, actual asset placement, complete runtime/firmware link, whole-engine/global-FX sound, current Juno coexistence, CPU, native fit, device or release acceptance follows.


## Runtime-only immutable direct-word read seam (candidate)

This successor keeps the original `S1NoteState::run`, original C morph function, original oracle and common declarations byte-identical. Only Scheduler's mono/poly render sites may select a generated free runtime adapter. The adapter binds the actual note fields by reference and derives the original private helpers/body from pinned input, redirecting only its two morph calls. A separately named C derivative retains the original float interpolation/phase arithmetic and replaces the four table loads with checked little-endian word reads and four-byte bit copies. No global/TLS reader, mutable shared cache, arbitrary callback, base-to-derived cast, virtual dispatch or heap owner is introduced.

`reader/generate.py` authenticates both complete original source files and records checked body/helper/output identities in `reader/DERIVATION.json`. Host/native runners independently regenerate and compare the compiled owned files before building. RuntimeState owns a small binding copy over the caller's immutable bytes; only the fixed exact row map is admitted. Structural finite/range/guard admission is not a cryptographic authentication of canonical public content: the fresh runner separately pins the public corpus. Sine and tuning remain direct and separate.

`Scheduler::init_reader` retains the real raw `Tables` pointers in this first slice and requires their exact packed mapping. It validates full owner/cross-span/count/alignment/mapping/domain rules before changing live state. Failed admission preserves existing binding/ports/DSP. Raw reinitialization clears reader mode; reset/restore retain it. Render outputs cannot overlap the actual packed span or any field of the full final Scheduler. Borrowed byte lifetime/immutability remains the caller's contract. The direct backend is total on admitted immutable bytes and in-domain indices. An unexpected internal read fault returns process failure; no atomic rollback of an already advanced voice is promised and no zero/old sample fallback is accepted as success.

The new fixture uses the unchanged distinct-row original oracle. It asserts actual reader-path calls and zero oracle reader calls, keeps the #942 real warmup/alias/release/PCM/state controls, checks every logical word at all three rates, and covers invalid admission, late-derived-field output overlap, old/new-mode reinitialization and independent live instances sharing immutable bytes. Component tests also run the new C morph function with all raw runtime descriptor payload pointers NULL; that proves only the primitive's load source, not Scheduler admission of a compressed/metadata-only bank.

Producer O2/Os reader metrics: 10 cases, 1,701 positive frames, 2,580 snapshots, 54 events, 639,132 reader-word comparisons, 722,244 reader calls, zero oracle reader calls, 7,371 oscillator cases, 7 protected writes, 189 releasing-alias calls and three scheduled wrong-data controls over 1,536 frames. Both legacy O2/Os fixtures retain their earlier exact metrics. Default LeakSanitizer fails under ptrace; the exact same sanitizer binary passes ASan/UBSan/float-cast-overflow with leak detection disabled and empty stderr. These are candidate results to be independently rechecked, not target acceptance.

The standalone C morph comparison covers 9,477 cases including positive wtpos wrapping and 37,908 actual reader calls, with bitwise output/state equality to the unchanged original C function. Direct-reader admission tests compare all 213,044 logical words and reject 126 malformed/alias/range cases, including a bad guard/nonfinite value in every unique row. Mutation proof separately distinguishes scheduled finite/audible original PCM witnesses from admission/lifetime checks; compile/link failures never count as detected runtime faults.

The genuine pi32v2 r3/mfprev1 profile compiles 20 runtime objects. Its compiler-evaluated Scheduler/RuntimeState size is 9,972 bytes, with a 12-byte direct binding/alignment 4. References/lambdas/helper duplication may change native code/stack cost; no optimizer-removal or free-resource claim is made. The old 9,956-byte reservation diagnostic is historical, not a new fit result.

This first seam still requires the raw 819,400-byte morph payload plus separate 16,388-byte sine. It does not implement a codec, eliminate raw storage, place assets, complete a native executable/firmware link, supply platform I/O, qualify current Juno coexistence, prove target DSP/CPU/workload/stack/fit, whole-engine/global-FX sound, device or release. All previous target gates remain open.
