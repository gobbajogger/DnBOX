# Independent experiment diagnostic review

**Judgment:** No actionable findings within the bounded diagnostic scope. This judgment accepts the accuracy and limits of the diagnostics, not Synth One firmware integration.

Frozen input: 20 files, manifest SHA256 `d138f3e30de8983ed096dee4c0fa8d649365d29ccc7ceb26e7d7a8a2c76c9cdd`. All listed bytes and hashes were independently verified and remain unchanged. Accepted #941 source: `bb6745d6f794d4befe6d03610e51400891bb25e1`, source-manifest SHA256 `d138334eb8a6eb800b1022a8d58b828aafe149d2408d1e3977184bdd46c8d58c`.

## Supported results

- Preserved #869 baseline `94197fc839b00aff47b98ca02f3c6396e4a6eabf`: Elite **PASS**, Pro **PASS** for their recorded native build/check gates. Source identities, retained output hashes and relevant ELF layout were independently checked.
- The resource-only per-part union reservation retains existing engines and metadata. Both Elite and Pro links **FAIL**: the 12,584-byte two-part growth exceeds the unchanged pool by **4,244 bytes** and overlaps the C1 stack. Baseline pool headroom is 8,340 bytes against the unchanged 8,192-byte floor. The reservation supplies no Synth One construction, code, tables or sound.
- Original runtime executable link: **FAIL**, unresolved `__assert_func`. Separate nonreturning assertion-hook diagnostic: **FAIL**, unresolved `fprintf` in the original morph initializer. Neither produced a complete executable; no platform, printf or DSP substitute was used to manufacture a pass.
- Historical type-probe identity is reconciled: its receipt names earlier supplier manifest `08879ca092f68825d088006df46cf36bd1544d6953157876aff783c986aa8a7f`, but its Scheduler header matches accepted #941 byte-for-byte. The accepted native layout independently confirms **9,956 bytes, alignment 4**.

## Limits

The pool rejection applies only to this direct reservation on the preserved baseline. It is not a current NIGHTLY fit result or rejection of every allocation strategy. No builds or links were repeated during this independent review. No current Synth One firmware or complete native acceptance, completed-image no-soft-double result, table-storage/decode fit, CPU measurement, DSP fidelity, target execution or device acceptance is established. Later work is outside this frozen review. No shared candidate or gate was changed.
