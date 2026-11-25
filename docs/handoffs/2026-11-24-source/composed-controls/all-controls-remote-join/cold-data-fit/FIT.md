# Seven additional immutable coefficients in exact guarded fit

Starting from the combined archive + UI + FAST32 + RAM variant
(27,948/27,940 B overflow), generate native byte receipts for the selected
existing numeric tables, then add them to the existing read-only Slot 3 and
FITTAB lists. No extra region, no sample changes, no XIP or RAM cap edits,
and no live BSS layout changes. Both actual guarded links save exactly
2,700 B of XIP; Elite still exceeds by 25,248 B and Pro by 25,240 B.
The map-only reject offers no runnable ELF, so the final paired guard
byte-check and real timing remain unrun. Remains a never-install lab.

Extra tables occupy 1,800 B in Slot 3 and 900 B in FITTAB. The native-byte
receipt script compiles only private source constants in a separate tiny
object to determine expected values, plus the current matched emitted
native velocity section. It writes hashes only as the portable receipt.
Do not include source coefficients, object bytes or audio in this branch.
After eventual fit the existing link harness must compare the actual
placed bytes to all pinned hashes, seal the data pair, and run the full
timing, including noise advance/rollover and active effects.
