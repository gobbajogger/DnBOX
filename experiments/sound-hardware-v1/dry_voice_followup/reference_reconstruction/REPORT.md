# Original-only dry baseline reconstruction

## Result

**The reconstructed original matched the required baseline byte for byte.** The one authorized capture of `physical-gain/100_2000_.9` produced exactly **1,058,304 mono bytes**, SHA-256 **`b02703f0893f9e7676e4db03048f9fefe8575ae1b1a1f44da1167253309a6cd2`**.

The attempt is terminal and its host build/render allocation has been released to root. No active-modulation capture ran. No source repair, retry, alignment, gain fitting, normalization, tuning adjustment or threshold change occurred.

The exact identity result is [metadata/baseline_identity_result.json](metadata/baseline_identity_result.json), SHA-256 `287675dcf9e9428d05e908fed75aefcb2616a9eae4dbf812d429cbfcfe4a2150`. The complete bounded-attempt summary is [metadata/authorized_attempt_summary.json](metadata/authorized_attempt_summary.json), SHA-256 `8d43d40aef503943f5649671e883b24678c8a5196210c214ec4275775e9ec298`.

## Executed scope

Root explicitly authorized frozen proposal SHA-256 `36d914226f921180f3c7d79643105e900650f65d9d3a1c5016e496891cb57aed`. The original preparation snapshot remains unchanged in [PROPOSAL.json](PROPOSAL.json).

| Stage | Authorized maximum | Actual outcome |
| --- | ---: | --- |
| Reference translation-unit compilations | 17 | 17 succeeded |
| New public wrapper compilation | 1 | 1 succeeded |
| Host link | 1 | 1 succeeded |
| Read-only link/runtime/symbol audits | 6 | 6 succeeded; admission checks passed |
| Selected baseline capture | 1 | 1 succeeded and matched exact output identity |
| Active-modulation capture | 0 | 0 |
| Retries or source repairs | 0 | 0 |
| Native target link, emulator, suite, timing benchmark, device or RDC operation | 0 | 0 |

All compiler-driver commands ran serially. The combined compile/link phase was constrained by one 60-second deadline, with shorter per-command caps. The baseline process had its own 15-second limit. Those bounds governed execution; this was not a timing or real-time qualification.

## Exact baseline scene and observed counts

The scene uses 44,100 Hz, velocity 100, requested cutoff 2,000 Hz and binary32 resonance 0.8999999761581421. The full 150-value physical control vector is frozen from the existing case plan. Notes are 36, 60 and 84, each with a fresh original instance and exact recorded tuning.

| Recorded quantity | Actual |
| --- | ---: |
| Notes completed | 3 |
| Silent warmup frames, all notes | 529,344 |
| Retained frames, all notes | 264,576 |
| Total reference frames | 793,920 |
| Original process calls | 12,405 |
| Frames per process call | 64 |
| Note-off frame within each retained note | 66,112 |
| Warmup nonfinite samples | 0 |
| Retained mono nonfinite samples | 0 |

The retained stream is the original left mono output. The raw file is [outputs/physical-gain_100_2000_r0.9.original-mono.f32le](outputs/physical-gain_100_2000_r0.9.original-mono.f32le). It matches the already recovered original column exactly; it is not a candidate-engine comparison.

## Wrapper, input and executable identities

| Item | Bytes | SHA-256 |
| --- | ---: | --- |
| New public-API/file-I/O wrapper | 9,727 | `9ddfb70be9b9d26cc7d2b27f20ce228aeab197c3a9f8a65decc5c42a99f5a2b4` |
| Successfully linked isolated host executable | 63,936 | `edf9f733ff449ecf345f9843952dea5d7cdab00825de88e4d7c3c61e6135f91d` |
| Authorized proposal | Recorded in proposal manifest | `36d914226f921180f3c7d79643105e900650f65d9d3a1c5016e496891cb57aed` |
| Post-link admission audit | 3,377 | `fca2abdb8474d021383e0734f562801cf0c530a53edfb39966d844bf8c499cf2` |

The wrapper implements fixture loading, public original calls, event boundaries and file I/O. It contains no voice or filter algorithm. Its source is [capture_original_dry.cpp](capture_original_dry.cpp).

All **39 original input identities** were staged into this isolated comparison directory and verified before execution. They comprise 37 source/support files plus the public table input and numeric case plan. Two 600-byte control vectors and one 512-byte tuning vector were derived as numeric fixtures; no audio was generated during preparation. The three played tuning identities were checked against the existing case plan. The exact inputs and derived numeric fixtures are recorded in [metadata/staged_inputs.json](metadata/staged_inputs.json) and [metadata/numeric_fixture_manifest.json](metadata/numeric_fixture_manifest.json).

The historical missing scheduler-object and header identities were not recovered or silently substituted. All selected translation units were rebuilt together from one consistent recovered source tree. The exact selected baseline result is the admission evidence for this reconstruction. It does not prove every other case in the historical matrix.

## Admission and environment checks

The execution guards used explicit conditional checks and exceptions, with no removable assertions. They checked the root-authorized proposal hash, wrapper, all 39 staged inputs, all three derived numeric fixtures, tools and runtime identities. They rejected nonempty unpinned include/library/tool injection variables, including `LD_PRELOAD`, and used an explicitly recorded child environment. Current local compiler/renderer capacity was observed and serialized before build and capture.

The completed audit confirmed:

- Required original processing/note definitions were present.
- Candidate scheduler, source-scheduler, compact-reader and runtime candidate symbols were absent.
- Exactly the declared 18 workspace objects entered the link.
- Dynamic dependencies were within the declared runtime set.
- Every resolved non-vDSO runtime file matched its pinned identity.
- The executable is a host x86-64 image.

All seven admission checks are true in [metadata/authorized_link_audit.json](metadata/authorized_link_audit.json). Tool/runtime identities and the explicit execution environment are preserved in the metadata directory. Every attempted command retains its own stdout, stderr and receipt. Original-source compiler diagnostics stay with the isolated reference owner because raw diagnostics can contain source excerpts. Only hashes, public build metadata and numeric results should be passed to independent implementation workers.

## Preservation and limits

The eight completed standalone-filter outputs, their input files and observation packet, and the six depth-zero alias identities were verified unchanged before and after this attempt. No historical failure record or acceptance threshold was edited.

This is one exact reconstruction check for three notes. It establishes no new independent-engine pass, active-filter behavior result, factory-program coverage, native memory fit or real-time qualification. The exact source closure is comparison recovery evidence only; it is not an implementation dependency for the independent engine.

Root has released its separate native sequence. No further host build or render will start here until root gives a terminal native release and an explicit new allocation. A single exact predeclared active-filter case can be proposed separately, using this successful executable and its unchanged public wrapper. It must be labelled a **new original-reference capture**, not recovery of the missing historical paired bytes.
