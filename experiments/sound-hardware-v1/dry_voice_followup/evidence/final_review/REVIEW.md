# Independent final review: D static results and stock native resource evidence

## Verdict

**No blocking reporting or accounting defect found in the completed saved evidence.** The public D/static and stock-link conclusions are supported at their stated scope. D's complete static fidelity gate remains **FAIL**; the stock resource links do not establish target sound, timing, complete instrument coverage, production activation, or full-lane admission.

This reviewer performed read-only report, receipt and physical-map analysis. No compiler, render, native tool, emulator, GitHub operation, Page write or canonical edit ran. No original engine implementation, header, object or diagnostic was opened. Original numeric PCM was not needed.

## D: the actual 180-note outcome is supported

I reconciled all **60 saved paired reports and 60 receipts**, checking the report hashes, paired-recording hash references, gain, finite counts, per-note thresholds and per-file acceptance. The independently recomputed rows exactly match the saved summary and its canonical copy:

| Measure | Verified result |
| --- | ---: |
| Passing notes | 138 / 180 |
| Passing files | 45 / 60 |
| New passing notes relative to C | 29 |
| Previously passing C notes lost | 0 |
| Failing notes | 42 |
| Maximum RMS error | 4.832778298315766 dB |
| Maximum spectral error | 8.991579577623538 dB |
| Nonfinite output reported by every case | 0 |

Acceptance remains gain 1, RMS error at most 1 dB, spectral error at most 3 dB and no nonfinite output. I did not rerun or alter the metric. The root's `STATIC-ANALYSIS.json` counts and all five failure groups agree with the actual saved reports:

| Cutoff | Resonance | Failed notes at each velocity 20 / 100 / 127 | Total failures |
| --- | ---: | --- | ---: |
| 64 Hz | 0.9 | 36, 84 | 6 |
| 8,000 Hz | 0.5 | 36, 60, 84 | 9 |
| 8,000 Hz | 0.9 | 36, 60, 84 | 9 |
| 22,050 Hz | 0.5 | 36, 60, 84 | 9 |
| 22,050 Hz | 0.9 | 36, 60, 84 | 9 |

The selected `physical-gain/100_2000_.9` report passes all three notes. The README correctly preserves the complete gate as FAIL and says production Synth One is still silent and unresolved. It does not turn the 29 improved notes into a full original-sound claim.

The separate D source review is correctly presented as readiness for measurement. Its counts are 15 source/support identities, three recorded binaries, eleven test-evidence files and a 33-entry D manifest. I suggested changing “33 original manifest entries” to **“33 frozen D evidence-manifest entries”** to avoid confusing these with original PCM observations; the root reports adopting that wording.

The supplied D review and root analysis record an unchanged zero-resonance branch and 15 identity rows. I did not independently reopen/hash all B/C/D PCM or complete compressed evidence bundles in this final pass. That raw identity check is therefore attributed to the existing evidence, not claimed as a new reviewer execution.

## Stock Elite and Pro: physical-map arithmetic agrees

I parsed each of the four saved physical maps independently. The load calculation uses the maximum flash load-section end minus XIP start `0x02000120`, rounds upward to four-byte alignment, then includes the stipulated four-byte image marker. These maps need zero additional alignment padding. Physical XIP capacity remains **552,672 bytes**.

| Frozen stock image | Mapped load extent | Marker | Load including marker | XIP free | Above 1,024-byte reserve |
| --- | ---: | ---: | ---: | ---: | ---: |
| Elite reference | 551,488 B | 4 B | 551,492 B | 1,180 B | 156 B |
| Elite shared | 544,704 B | 4 B | 544,708 B | 7,964 B | 6,940 B |
| Pro reference | 551,480 B | 4 B | 551,484 B | 1,188 B | 164 B |
| Pro shared | 544,696 B | 4 B | 544,700 B | 7,972 B | 6,948 B |

The resulting saving is **6,784 bytes on each stock model**. This agrees with the `.text` difference: Elite 547,572 to 540,788 bytes; Pro 547,564 to 540,780 bytes. It is separate from the earlier 6,496-byte compact Elite result and cannot be added to it or substituted for an unmeasured compact Pro result.

All map section dictionaries match their raw `RESULT.json` values. The RAM arithmetic is also exact across the four jobs:

- Pool: 335,092 of 344,064 bytes used; **8,972 bytes raw free**, 780 after the 8,192-byte reserve, and only **268 bytes beyond the additional 512-byte requirement**.
- Ordinary RAM: 97,204 of 98,304 bytes used; **1,100 bytes free**. The extent includes 600 bytes of data, 96,596 bytes of BSS and eight bytes of intervening alignment.
- XPOOL: 9,216 of 9,216 bytes used; **zero bytes free**.

These are the frozen stock development-model-900/default-empty-BREAK profiles. The independent S1 dry voice and compact research core do not enter these resource conclusions. No package/image assembly or target firmware execution is inferred from map accounting.

## Saved execution evidence supports native build completion

Each plan declares 73 compile units and 357 frozen input manifest entries. I checked all **308 saved command statuses**: 292 compilations, four physical links and twelve inspections, all exit zero. All 296 compile/link argument lists match their exact plans. Saved command intervals and outer job intervals are serial; every terminal receipt reports its owned local process group released. Plan/map/command/result hash links reconcile.

The raw historical `native_execution: NOT_RUN` field is retained. The current aggregate and README explicitly distinguish completed native **compiler/linker/inspection execution** from **target firmware code execution: NOT_RUN** and emulator execution: NOT_RUN. That distinction is accurate. The symbol/disassembly claims were reviewed as supplied aggregate evidence; binaries and full disassembly were not reopened.

The root reports publishing the stock evidence at `f291e916a9b61ae862899f0d26e21d42d76a7846`. This review did not query GitHub and does not independently certify publication state.

## Exactly one known pending future comparison

The **one active candidate comparison remains NOT_RUN**. The README placeholder was a known in-progress item, explicitly excluded from surprise-blocker reporting. The completed original-only active capture does not establish a candidate match. This report closes the D/static/native saved-evidence review; a bounded addendum can assess the active claim after the actual comparison.

The owner instructed this reviewer to finish without reopening the full compressed manifests or raw zero-resonance recordings. No optional test expansion or new execution is required by this review. `REVIEW.json` records the detailed numerical checks, limits and identities of the reviewed inputs.
