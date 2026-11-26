# Quarantined SuperJ6 first-note physical-cutoff PI32v2 A/B
All arms use the already recovered app source with the exact Synth One + Remote + arp lab donor. No physical device writes, no package produced, no publishing, release-anchor and ordinal gain patches absent. `run.sh <0|1> <elite|pro>` compiles the complete actual target app with on/off only. Preimages match at fuzz zero, offsets only. These are lab memory figures, not ship/release approval.

| Model | XIP used off/on | XIP free off/on | Pool usable off/on |
| --- | --- | --- | --- |
| Elite | 552,408 / 552,312 | 264 / 360 | 564 / 540 |
| Pro | 552,396 / 552,300 | 276 / 372 | 564 / 540 |

The -96 B code/layout result includes final link relaxation/LTO; do not use it as portable added code prediction. The active image uses 24 B more of real pool. POOL_MIN is 512 B, so 28 B cushion on this source only. FDATA, FITTAB, slot3 usage and plain static RAM/end are unchanged. PI32v2 layout compilation shows juno6_t 3,884→3,896 B, original_runtime_part 12,212→12,224 B, core 2,936 B and workspace 5,120 B unchanged. The two-part pool object matches +24 B. Prologue witnesses show 500 B typed-span local allocation unchanged; render wrapper 68→76 B, PWM 40→64 B (both helper register saves unchanged), outer juno renderer unchanged, runtime key prelude stores one extra register. These are local compiled frame sizes, not full high-water or audio gate results.

`replay.sh` materializes a sealed disposable simulator-only image and takes the existing emulator lock. Both builds retain the real typed owner and loaded S1 arp. The heavy integrated input is an interoperability test, not a performance qualification; use the per-arm logs as raw status. This does not reach qualified exact desktop #180 preset import or full native relatch signal under a controlled matched-source clip; those still need actual full production pathways.
