# Independent review of the executed Sound native pair

## Decision

**The exact Elite resource comparison is valid.** The reference physically fails; the shared-fastmath candidate physically links and clears the unchanged XIP and pool limits. I found no source-level arithmetic, ABI, state ownership or memory-region reason to reject preservation of this header optimization for further qualification. It is not yet a production audio/timing acceptance or a completed Synth One/SuperJ6.

I performed read-only source, manifest, command, map, symbol and disassembly inspection. I ran no additional build, Pro link, emulator, test or benchmark.

## Independently recomputed physical results

| Measure | Reference | Shared candidate |
| --- | ---: | ---: |
| Compiler exits | 74 × 0 | 74 × 0 |
| Physical link exit | 1 | 0 |
| `.text` including rodata | 549,500 B | 543,004 B |
| Aligned load image plus 4-B marker | 553,420 B | 546,924 B |
| Physical XIP free | −748 B | 5,748 B |
| Margin after 1,024-B XIP reserve | −1,772 B | 4,724 B |
| Ordinary RAM extent | 97,204 B | 97,204 B |
| Ordinary RAM free | 1,100 B | 1,100 B |
| Main pool used | 335,092 B | 335,092 B |
| Main pool free | 8,972 B | 8,972 B |
| Margin above 8,192-B floor | 780 B | 780 B |
| Margin beyond required 512 B | 268 B | 268 B |
| XPOOL free | 0 B | 0 B |

The candidate reduces the real image by **6,496 B**, entirely in `.text`; every other section retains its byte size. The reference linker reports overflow of 744 B before the four-byte image marker, consistent with the final image overflow of 748 B. No diagnostic map enlargement was used. Candidate ELF SHA-256 is `4d70c3c92ba2b9f1159dab1c4390909a692237b7dda7621418898030b78a6d36`.

All 359 source hashes per plan remain unchanged. Every actual compiler argv equals its prepared argv, and every actual link argv equals the prepared one. The reference command sequence ran 23:04:27.088217–23:04:40.839045 UTC; the candidate followed 23:04:50.334261–23:05:03.983956 UTC. They were serialized. Raw map/result/command hashes are retained in `FIT-LINK-AUDIT.json`.

## Shared function and ABI audit

The patch changes exactly six declarations, after removing comments and whitespace. Their expressions, constants, argument types, return types, guards and local variable definitions are unchanged. These helpers own no mutable static state, pointer-owned history, allocator, event or part state. Sharing their code creates no shared voice or FX state. No alternative definition or incompatible prototype for these names was found in the compiled source closure.

The candidate retains one named definition of each family:

| Function | Binding | Bytes | Selected source |
| --- | --- | ---: | --- |
| `fm_exp2_nf` | Weak global | 102 | `dsp_drum909.o` |
| `fm_exp2f` | Weak global | 54 | `dsp_drum909.o` |
| `fm_expf` | Weak global | 92 | `dsp_drum909.o` |
| `fm_log2f` | Weak global | 154 | `dsp_drum909.o` |
| `fm_logf` | Local after LTO | 18 | LTO object |
| `fm_db2lin` | Weak global | 14 | `dsp_fxbus.o` |

Other copies appear in the map's discarded input sections. The output has no named undefined symbol and no detected software-double routine. All 70 C compilation units retain `-mcpu=r3`, `-mfprev1` and `-ffp-contract=off`; no unit enables fast math or a different FP contraction mode. The linker likewise keeps FP contraction off. The four assembly units are unchanged. Mixed LTO and non-LTO callers have all been recompiled with the same declarations and resolved to a single C-signature implementation.

No reference to the newly shared helpers appears in `.ram_text` or `.c1_text`; their section sizes are unchanged. No new flash-sensitive RAM path to these XIP helpers is evidenced.

## Why the saving exceeds outlined-symbol deduplication

The previous standalone object result (+12 B) did not predict final image size. Much of the observed saving comes from replacing repeated inline expansions inside retained callers:

| Retained section | Reduction |
| --- | ---: |
| `d9_render` | 1,596 B |
| `voice_run` | 576 B |
| `d9_render_clap` | 364 B |
| `master_process_st` | 346 B |
| `fxbus_process_st` | 322 B |
| `d9_env_exp` | 312 B |
| `busfx_apply` | 290 B |

The complete input-section delta is recorded in the audit. The relevant engine callers remain present; this is not removal of whole engine owners, reduced voices or shortened histories.

## What remains before production-preserving acceptance

There is no newly discovered mathematical or ABI defect requiring a redesign of the header patch. Nevertheless, the compiler has changed native call placement in real audio paths. The retained X0X_HOST PASS exercises an unchanged static-inline macro, so it does not execute this native sharing path. The native disassembler also emits many unknown-instruction lines for floating-point encodings; its text cannot establish arithmetic equivalence by itself.

The next meaningful qualification must therefore use the existing independent native identity and callback/timing procedures for affected 909/808/BREAK, master/FX and new-core paths, with their existing thresholds. More host testing of the unchanged macro would not close that gap. Pro needs its own actual full-image result on the intended production base. Neither step was part of this already-consumed two-link allocation and neither was run here.

This fixture still has a stock18-control Juno facade and a silent Synth One production branch. Its fit leaves 4,724 B after the XIP reserve, but that space is not a prediction that the missing instruments/controls/FX will fit. State, memory, filter topology, audible fidelity, presets, complete controls, native timing and final `scripts/gate.sh sound --post` remain separate work. Preserve the useful header optimization and exact PASS/FAIL evidence; do not declare the lane complete or activate an unqualified renderer from this resource result.
