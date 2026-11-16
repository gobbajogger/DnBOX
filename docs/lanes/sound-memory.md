# Sound memory: component measurements, NOT all-engine fit

S6 remains FAIL. No final Elite/Pro combined link or runtime qualification exists.

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
