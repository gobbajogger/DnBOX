# Normal SuperJ6 WIDE on the compact wet renderer

The exact production-linked source is `1c46c6fc8d7a550b3308515a3415ea63d86e7486`, tree `b6d101bb3515d9a5fb29ada6214a2c91b57caaa5`. It incorporates Sound `2e6a69cc5229f8911b0158cae0f9a65356b41292`, production payload `bb836379`, and retains the WIDE preset, mixed-lock policy, and software Remote scalar. The separately compiled wet renderer and its header replace the obsolete `juno6_chorus_factory.inc` path. Parent history retains the old file. The stock production profile retains 808, 909, and BREAK at `-Oz` and uses the paired WIDE compile flags. Both model package and flashmap checks passed. Both actual packages passed the native emulated USB 909 MIX Level Apply and Undo flow and rejected a stale write after CLOSE.

| Normal WIDE scene | Free XIP | Pool margin | Calibrated eight-second peak | Late halves / xruns |
|---|---:|---:|---:|---:|
| Elite, WIDE A | 1,108 B | 1,244 B | 10,930 µs | 0 / 0 |
| Pro, WIDE B | 1,116 B | 1,244 B | 14,380 µs | 43 / 43 |

The attached fixture uses production selection of WIDE index 4 and normal project save/reload with pattern code E5. The exact package-matched programs use highpass/full-sub Synth One, both drum kits and an installed CRC-valid BREAK loop. Actual guest memory confirms the typed renderer, WIDE panel binding, positive wet mix and advancing mode-2 phase. Both scenes generated valid PCM, all five active parts, and no emulated guest fault.

Sound's 11,345 / 11,137 µs measurements used a separately instrumented typed row and different package and saved-project hashes. The Pro normal-WIDE failure applies to the exact software source here. The strict ten-minute run is NOT_RUN because the same normal candidate already misses the 11,609 µs deadline in the bounded preflight. The normal save/reload/locks suite passed 391 checks, and focused factory and legacy Juno checks passed. The prior untraced compact-program evidence carries on the exact ten unchanged source blobs. The firmware host used for paired runtime checks is identified by its attached SHA-256.

The compact, source-only receipts are retained here. Full raw audio, guest readback, virtual flash, and both packages remain in the local branch-and-source-named archive for the owner-authorized private Project Sources destination when capacity is resolved.

The independent real UI fixture passed 106 assertions across both WIDE owners. It verifies all four wave labels and distinct canvas fingerprints and the 50% Width ceiling. The parent UI negative failed on the old percent readout, and legacy readouts and four old glyph hashes remained identical. A direct decoder of existing guest RAM found Pro WIDE B changed stored Wave and Width from 64 to 127 by the end of the unshifted 922-event scene, with square/square engaged. Elite remained triangle/triangle. Both took the shared short-alignment chorus path. Pro's worker mailbox completed every assigned job with zero give-ups. These observations identify the candidate state for Sound recovery without attributing the misses to a single operation.
