# Independent review of frozen Variant C

## Verdict

**No implementation blocker was found in the frozen C translation of the centered-feedback proposal.** The state/history updates, six-step solve, unchanged zero-resonance arithmetic, physical domains, gain, and tail behavior match the declared experiment.

This is a source and saved-scalar-evidence review. It is not an audio PASS, a proof of arbitrary nonlinear or dynamic-filter stability, or production/target qualification. Later `results/audio180` filenames appeared during the read-only inventory; their contents and gate outcome were not inspected in this review. The declaration's `NOT_RENDERED` fields describe its preserved pre-audio snapshot.

Reviewed unit: `/workspace/scratch/e803955aec60/s1_dry_voice_c`.

| Frozen identity | SHA256 |
| --- | --- |
| `DECLARATION.json` | `adfae90f553d0dc159f8a70b34873cebe4bda3c82fa7f6e4412aa74b97e9a688` |
| `src/s1_dry_voice.c` | `29e04eb07c512bb390bf6f7cc7b9429baf46cf927db126bced2f1fffe1ae1bf9` |
| `src/s1_dry_voice.h` | `c18a55d812b5424b80fc611125d67d29b1297b59179ecb24b01f8807c3646ec2` |
| `src/host.c` | `068ed60874878844f65d0b90b4ac6b76597de3990962227f5ee74908ad0b7b07` |

## Translation and state correctness

**The proposal is implemented exactly at the arithmetic level reviewed.** After removing comments/whitespace and inlining C's private `feedback_solution`, C's `filter_substep` is identical to the preserved `inputs/centered_feedback_proposal.c` function. All four copied diagnosis inputs are byte-identical to `/workspace/scratch/e803955aec60/s1_resonance_diagnosis`. The helper extraction does not change operation order or iteration count.

**History indexing is correct in both branches.** `src/s1_dry_voice.h:34-38` declares two new per-voice floats. Before each internal substep, the four output lags are:

| Storage | Required value |
| --- | --- |
| `filter_history[3]` | `y[n-1]` |
| `filter_history[4]` | `y[n-2]` |
| `feedback_lag3` | `y[n-3]` |
| `feedback_lag4` | `y[n-4]` |

`src/s1_dry_voice.c:168-171,197-207` first preserves the old values, shifts `lag4 = old lag3` and `lag3 = old history[4]`, then moves old `history[3]` into `history[4]` and commits the new fourth-pole output. No current output is incorrectly reused as an old lag. Whole-object reset at lines 64-67 zeros the new fields. Updating them in the zero-resonance branch also keeps the histories coherent if a later configuration changes resonance.

**The implicit feedback equation is constructed from old state without premature mutation.** Lines 175-193 calculate the zero-feedback cascade result `q`, then form

\[
w=(q+4y[n-1]+6y[n-2]+4y[n-3]+y[n-4])/16,
\qquad c=(R/16)(1-A)^4.
\]

Lines 140-148 start at `d=w` and perform exactly six updates `d=w-c*phi(d)`. Lines 197-207 re-evaluate and commit each forward pole once using `input-R*phi(d)`. There is no data-dependent early exit. The source explicitly distinguishes the real-arithmetic identity from additional floating-point rounding.

**Zero-resonance arithmetic is retained.** Lines 157-172 preserve B's original nonlinear evaluation, multiplication by signed zero, forward arithmetic, and five original history assignments; only the two extra history writes are added. The oscillator/envelope/sample function remains identical to B, including the two held-input substeps and `0.5f*(first+second)` averaging at lines 230-236. Whole-note PCM identity is not claimed from the scalar checks alone.

**The intended boundaries remain unchanged.** Normalized source comparison found all 15 unchanged B helper/API/sample/render function bodies identical, including configuration, coefficient preparation, start/off/reset, oscillator/excitation, physical cutoff mapping, and block rendering. `src/host.c`'s renderer `main` is also unchanged from B. The three vendor files match B byte-for-byte.

The public physical ranges at `src/s1_dry_voice.c:79-97` remain cutoff 64–22,050 Hz, resonance 0–0.98, filter depth 0–1.2, and the existing ADSR/level/balance limits. The existing cutoff mapping and stock nonlinear saturation remain present. C introduces no new range restriction, fitted coefficient, output gain/postgain, tail cutoff, or voice-retirement rule. Its `R=0.8*physical_resonance` and pole preparation are inherited from B.

## What the tests actually establish

All saved result objects agree with their attempt-01 command receipts: exit 0, matching stdout JSON, and empty stderr. No test, executable, compiler, renderer, or metric was rerun by this reviewer.

| Evidence | Actual coverage | Limit |
| --- | --- | --- |
| Host `--describe` | Saved 160-byte voice, 92-byte prepared state, six expected envelope coefficient words | Host layout/coefficient metadata only |
| Host `--selftest`, `src/host.c:43-95` | Finite/range validation, nonmutating invalid configuration, endpoints, squared velocity, explicit unsupported retrigger, oversize-block rejection, gate-copy independence, reset of the two new lags | No sample loop, block-partition audio test, or full voice trajectory |
| 38 scalar solver points, `tests/scalar_test.c:52-63` | Actual private solver output matches independent numeric fixture words; finite output and root-bracket checks | Selected arithmetic points, not exhaustive binary32 or audio proof |
| 360 zero-resonance substeps, `tests/scalar_test.c:65-99` | Five cutoffs × two signed-zero resonances × three initial histories × twelve fixed scalar inputs; output and all five original history words match the frozen B comparator | Scalar substeps, not a whole-note PCM comparison |
| 720 nonzero-resonance state checks, `tests/scalar_test.c:101-137` | Five cutoffs × four resonances × three initial histories × twelve scalar inputs; selected contraction bound, finite/bounded output, and exact lag shifts | No original-engine comparison, long-tail qualification, or dynamic-filter proof |

The 38 header fixture triplets exactly match the binary32 words of the independent diagnosis's stored `w`, `c`, and six-step results. The scalar executable includes the actual candidate translation unit (`tests/scalar_test.c:4`) rather than a separately reimplemented C solver. Its frozen B comparator is confined to the test executable and reproduces the permitted B zero-resonance arithmetic.

**Coverage note, not a required code fix:** the `untouched` object's comparison at `tests/scalar_test.c:68-70,132` is not an interleaved two-active-voice ownership test; that object is never processed. Describe these 720 iterations as history/state checks. Per-instance ownership is also supported by the source's local `voice` state and the host gate/reset checks, but no broad dynamic ownership claim follows from that one `memcmp`. The existing `READY.md` and saved scalar receipt already use appropriately narrow history/state and no-audio wording.

The saved maximum selected float contraction, `0.0964563638`, remains a scalar arithmetic result. Neither it nor the diagnosis's small-signal derivation establishes original S1 sound fidelity. The declared gate remains gain 1, RMS ≤1 dB, spectral ≤3 dB, and zero nonfinite samples.

## Freeze, isolation, and import

The audit matched all **17 source/support files, 3 compiled files, and 10 test-evidence files** against the final declaration, plus the pretest declaration identity and all three named source-free packet hashes. The final source freeze agrees with the pretest source freeze. `C-AUDIT.json` records each checked digest and the structural comparisons.

`build.sh:5-16` builds only the standalone candidate/host and separate scalar executable. The host build does not include the test comparator. The candidate adds two floats, **8 host bytes**, for a total of **160 bytes**; prepared state stays 92 bytes. There are no added allocations, mutable globals, original-engine dependencies, production registration hooks, or firmware build changes in the reviewed unit. The saved core undefined-symbol list names only the local envelope configure/sample functions. This is isolation of the reviewed standalone unit, not a repository-wide production-link audit or a native fit claim.

Root can import the exact six source files from `COMPARATOR-SOURCE.json`, preserve this declaration and test evidence, and register the expected host size of 160 in the portable manifest. The existing scalar results must remain separately labeled from the authorized 180-note comparison. No source change, new candidate, additional compiler run, optional rerender, or new test sweep is required by this review.

## Review scope

Read-only inspection and bounded Python processing of allowed source, saved metadata, fixture values, and file hashes only. Zero new compiler, scalar-executable, renderer, or metric calls. No original S1/SJ6 implementation, tables, source-aware candidate, native target, emulator, whole-engine gate, RDC, device, or GitHub operation was used. No canonical or C-unit file was edited.
