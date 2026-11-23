# Stock Elite/Pro physical-map links completed; target sound remains unqualified

The four exact stock plans ran in the order allocated by
[PR973 comment6995594729](https://github.com/gobbajogger/DnBOX/pull/973#issuecomment-6995594729)
on the separate local executor. All **292 serial compilations, four physical
links and twelve inspections** completed with exit 0. Each job rechecked all
357 frozen inputs, native tools, headers/resources, environment and local
capacity. All four owned process groups were released. This allocation is
consumed; no RDC fallback, target execution, loader/package, timing, full
suite or gate ran.

| Stock image | Load including four-byte marker | Physical XIP free | Above unchanged 1,024-byte reserve | Link exit |
| --- | ---: | ---: | ---: | ---: |
| Elite reference | 551,492 B | 1,180 B | 156 B | 0 |
| Elite shared | 544,708 B | 7,964 B | 6,940 B | 0 |
| Pro reference | 551,484 B | 1,188 B | 164 B | 0 |
| Pro shared | 544,700 B | 7,972 B | 6,948 B | 0 |

Sharing saves **6,784 bytes of .text on each stock model**. This is a separate
measurement from the earlier compact-core Elite saving of 6,496 bytes. Do not
add them or substitute a stock result for unmeasured compact Pro fit.

All four maps retain pool usage 335,092 bytes, raw free 8,972 bytes, 780 bytes
above the 8,192-byte reserve and only 268 beyond the additional 512-byte gate
margin. Ordinary RAM uses 97,204 bytes with 1,100 free; XPOOL uses all 9,216
bytes. There are no named undefined or soft-double symbols and no call
instructions in .ram_text. These are resource observations, not complete
instrument acceptance or target behavior/timing proof.

## Source identity and actual evidence

The plans retain source commit `e0ef1994a91abef619c755f9428e44c8c9f064c4`,
tree `9d90b64c610126c5602a12e2409bc60305f4a02d`. At allocation/execution the live
published lane was `f7f27cde98f7ac7594eb4ad23526652f0fb10130`; all 351 selected
canonical files matched that published tree. The planned reference reversal
of six fastmath linkage declarations remains explicit. Consolidated was
`76040daa5b52d686a8b7b544296ba5d79610a95d`.

These official 73-unit closures contain the stock Juno facade, Chicago/Miami
and still-silent Synth One. The independent S1 envelope/dry voice and compact
research core do not enter them. Development model identity 900 and the
empty default BREAK bank are frozen; no release/factory-payload fit is inferred.
The exact physical script is unchanged and its XIP capacity stays 552,672 bytes.

`execution/` contains the actual allocation, source reconciliation, aggregate
report, per-job RESULT/admission/terminal records and four lossless
`maps-and-commands.tar.gz` archives. Each archive includes the physical map,
all 73 compile logs, command receipts, physical-link log, symbols and sections.
`EVIDENCE.json` pins every member and the full disassembly/ELF identities.
Complete disassembly, ELF/object files, frozen source trees and execution
inputs are retained in the recovery bundle. The old RESULT field
`native_execution=NOT_RUN` is preserved unchanged; actual compiler/link and
inspection execution is explicit in the terminal records. Target code itself
and the emulator were not run.

## Preserved preparation and execution boundary

The preparation scripts, four compressed plans and guard regression records
remain unchanged. `NEXT-PRODUCTION-PLAN.md` and `PREPARATION-STATUS.md` describe
the earlier preparation state; this current report and `execution/` supersede
their NOT_RUN/allocation-pending status without editing historical evidence.

The scripts retain their historical absolute paths and hashes. Do not run
these Git copies or silently rewrite consumed plans. Restore the full frozen
recovery package for inspection. Any new execution requires its own concrete
scope and fresh guards; these four plans already have terminal receipts.

The corrected entry is `verify_inputs.py --execute --plan EXACT_PLAN`, which
checks all inputs and enters the historical executor in the same unoptimized
interpreter. Direct historical execution is superseded. Both wrappers reject
optimized Python and use explicit mandatory checks. Five compiler-free guard
negatives and four preparation input checks are retained in `guard_fix_01`.
`PLAN-ARCHIVE-HASHES.json` pins both compressed and original plan bytes.

## Exact remaining target-identity blocker

The available diagnostic pins old firmware 94197fc8, compares observer-off/on
on the same image, and omits 808 from its representative scene. It cannot
qualify the new reference/shared pair. The host identity runner uses
X0X_HOST/static-inline math, so its success would not test native sharing.

Next target qualification needs exact successful image/package pairs and a
reviewed driver covering 909, 808, accepted nonzero BREAK, FX and master, with
aligned PCM and normalized relevant state/event comparison. Fresh flash must
preserve each image's own code bytes. No such package/driver pair or target
run is accepted here. Full timing and `scripts/gate.sh sound --post` remain
unqualified and unrun. Production Synth One is still silent.
