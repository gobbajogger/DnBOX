# Sound memory: component measurements, NOT all-engine fit

S6 remains FAIL. No final combined Elite/Pro link with the required complete
Synth One/SuperJ6 replacements has been qualified.

## Latest Sound component checkpoint: 3e41

Source: `3e41e4b37c91a8f30542f3cd0724c6a518d588fd` (runtime unchanged in the
later documentation-only checkpoint `103286c8fa38f908c23e409b81ab37b6ee084f8d`).
The 31 unlinked native runtime/adapter units total:

| Section | Bytes |
|---|---:|
| Executable | 39,392 |
| Readonly | 4,522 |
| Writable | 616 |
| BSS | 56 |
| Separate compact waveform pack | 165,292 |

The dummy layout object is excluded from those sums. Its assertions establish a
7,972-byte pre-FX owner aligned to 4 bytes, not an additional allocated owner in
an application. Borrowed tuning storage and all application lifetimes still
need explicit combined accounting. The observed morph/quad/physical-row stack
prefix is 316 bytes; deeper callees, callers and interrupt/library frames are
not included, so it is not a complete stack bound.

The original full-FX delay allocation remains 7,680,016 bytes. The separately
audited reachable-delay estimate is about 2.88 MB; neither figure is admitted
stock-memory usage. No delay/control range, engine, protected flash region or
musical output was removed to obtain these component measurements.

Evidence: `docs/sound/v2-morph-bitcopy/native-sections.json`, its stack-prefix
receipt, and `docs/sound/v2-quad-distinct/native-summary.json`. Full command/dependency receipts remain privately preserved; their public upload
is blocked and is not claimed here. No all-engine
application link or S6 completion follows from these results.

## Existing-firmware baseline, not Sound admission

Integ reports the latest independently measured combined existing-firmware
source as `7e39818096e7fb728ef28cd6476b290b064e353e`: Elite/Pro XIP free
3,156/3,164 bytes, static RAM headroom 1,096 bytes, and pool margin 836 bytes above the
8,192-byte reserve. That is not qualification of its later published
`731f5abd8357cfe8654222b3d5e3ed03b5749fb0`, whose native remeasurement is pending,
and it does not include an admitted complete Synth One/SuperJ6 replacement.
The small independent baseline receipt is retained at
`docs/sound/v2-quad-distinct/integ-7e398-baseline-receipt.json`; its full Local
gate is explicitly NOT_RUN. No large build was repeated for this citation.

## Historical component checkpoints

The measurements below retain the development history. Their references to
"current" describe those earlier checkpoints, not the latest combined source.

## SynthOne pinned pre-FX owner

Adapter: `firmware/src/dsp/synthone_renderer.hpp`; source closure pin
56079bbdcb753c37d65aedd17ce49ba71c261797. Fresh input hashes and scope are recorded
in `docs/sound/v2-pre-fx-evidence.md`.

| Measurement | Bytes | Scope |
|---|---:|---|
| SynthOne pre-FX owner, native pi32v2 ABI | 10532 | Compiler-emitted `.size` only; no firmware link |
| Same owner, 64-bit host ABI | 12088 | Diagnostic host sizeof, not target budget |
| Compact immutable waveform pack | 165292 | Existing file size, not linked flash increment |

Native probe: `char synthone_pre_fx_owner[sizeof(dnbox::SynthOneRenderer)];`
compiled with existing jieli-linux-toolchains-20260730.1 pi32v2 cxx and its
pi32v2 include directory, `-std=c++14 -S`, source closure include directories,
and `firmware/src/dsp`. Output explicitly contains
`.size synthone_pre_fx_owner, 10532`.

Recovery: the retained host recipe used C++17, which this pinned compiler rejects.
C++14 was accepted; adding its matching target include directory resolved the
initial missing math.h. This compiles the type only, not the engine implementation,
so it does not prove all C++17 constructs have been removed from runtime sources.

No credit for removing old engines, waveform deduplication, or mutually exclusive
ownership is taken before exact linker and runtime evidence. Post-note FX memory,
source metadata, stack, program bindings, audio buffers and remaining engines are
not included in the 10532-byte owner. No PSRAM is assumed.

## Existing baseline warning

Historical baseline report `stock-memory-20261116.json` from prior sound work
reported pool used335700 of344064, free8364, only172 above the8192 floor. This
is below RULES v2's required additional512 margin by340 bytes before any new
engine. It is a warning, not current consolidated qualification. Integ owns a
fresh exact-source Elite/Pro link and shared engine memory planning.

Next: compile complete runtime with the pinned target toolchain, account for
post-FX and immutable table residency, and integrate only once combined memory
can retain ≥8704 free pool bytes. No final fit PASS is claimed.

## Fresh authenticated native runtime compile

Recovered and executed the pinned source's `direct-scheduler/run_native.py` with
`--full-sequence`, authenticated experiment/control dependencies, matching pinned
compiler, and original SDK C++ overload headers. Its native recipe uses C++11,
`S1_NO_DOUBLE`, exact integer binary64 clock helpers, and pi32v2/r3 FPU flags.
This supersedes the generic host-command native diagnostic, which did not select
that native closure.

Result: all29 runtime objects compiled; forbidden-symbol scan PASS,0 forbidden
symbols; native SourceScheduler10532 bytes, RuntimeState9984 bytes. Full details,
source/object hashes and dependency records: `docs/sound/v2-native-component-result.json`.
Absolute local paths there are normalized to named input roots. Original result
SHA256: `7e369ab97e2b11f49ce21b655f7ce9785d5fd739a3c29772ee7ab15c3e8806fc`.

Still NOT_RUN: native executable/firmware link, combined Juno/app resources, table
storage/read workload, CPU and device qualification. S6 remains FAIL.

Native baseline union probe at e742be7: original_runtime_backend3720 bytes, original_runtime_part5184, Chicago3492, Miami3720, Juno888. A simple10532-byte replacement union expands each backend by6812 bytes; two parts add13624 bytes before FX. This is not an accepted allocation plan.

## Serialized held-note snapshot reuse

The port's public events and render already require one serialized owner. Every
mutation synchronously assigned heldNoteNumbers to heldNoteNumbersAE before the
next use; the AE name now references that same Held object. This removes one
1536-byte backing array while retaining source spellings and event ordering.
No asynchronous publication, polyphony, note range, timing or DSP law is changed.

Native target size10532→8996 bytes, measured compiler symbol size0x2324; host
12088→10560. The intermediate host owner budget test failed before reuse and
passes after. Full original source-composition suite:38 cases,354907688 checks,
114196 frames,120289 snapshots,1081 control/note events,0 runtime heap calls,
0 failures. C/C++/oracle12-scene576000-frame comparison still has0 differences.
All29 runtime units were rebuilt from modified repository paths.

The test runner now overlays current repository runtime sources into a copied
verified fixture/oracle closure, records TESTED-SOURCES.json and compiles all
objects against that same staged view; it cannot accidentally test the old
retained runtime instead. Original provenance hash remains alongside changed
source hash in SOURCE-MANIFEST.json.

Integ is separately auditing reuse of the per-part Chicago4252-byte FX owner
with the3720-byte backend union (7972 total). Even if that passes its retirement
and tail tests, this8996-byte owner still exceeds it by1024 bytes per part,
before post-FX. No speculative reuse is credited toward S6.

## Flash remains a separate blocker

Current29 native objects before executable linking/dead-strip total33406B code,
3230B readonly,616B writable,56B BSS, excluding C ABI, sine/frequency metadata,
postFX and factory routing. Add165292B compact waveform pack. See exact per-object
hashes and allocation sums in docs/sound/v2-native-unlinked-budget.json.
Integ reports exact current app XIP free13464B Elite/13472B Pro; no fit credit
from protected map changes or external/user sample slots is assumed.

## Current pre-FX owner:7972 bytes

Held-note fields are admitted MIDI note/velocity0..127 and transpose-24..24,
so an aligned4-byte record (two uint8 fields,one int16) retains every valid
value. Invalid constructor inputs are marked invalid before casting, and both
insertion paths reject the marker. No wrapping values become valid events.
All128*128*49=802816 admitted triples are exhaustively checked.

Native owner8996→7972 bytes (symbol size0x1f24), host10560→9536. All29
runtime units compile again. Final full original suite:38cases354907688checks,
114196frames120289snapshots1081events,0failures0runtimeheap.576000-frame
C/C++/oracle test passes, and output bytes match the pre-alias5e541 run.

This equals the candidate per-part backend3720+Chicago FX4252 footprint, but
that is not combined fit: integ must prove retirement/admission and tails,
SynthOne postFX remains unadmitted, and flash is far over headroom.

Rejected alternative: compact-only two-descriptor build also changed the
shared raw oracle initializer and produced a failing comparison; it was fully
reverted, not hidden with an oracle edit. No descriptor-mode change is shipped.

## Transient source-helper API checkpoint

New scalar source-law setters do not add owner fields: native7972B/alignment4
static assertions still compile. Their local helper occupies248B; each emitted
wrapper saves20B registers before its callees. Complete worst-case call-stack
usage remains unmeasured. The frontend translation unit's unlinked symbols add
3414B T+104B W and1292B r. These are component symbol sums, not link-map size or
combined admission. Existing original fullFX/wave-table resource blockers remain.
See `docs/sound/v2-source-frontend-result.json` for exact source/result hashes.
