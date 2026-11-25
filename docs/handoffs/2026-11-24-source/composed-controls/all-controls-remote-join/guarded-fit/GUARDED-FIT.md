# Exact guarded fit, no runnable image

This uses the complete two-engine source trial in the previous compile
receipt, plus the archived guarded Slot 3 and FITTAB placement patches,
matching cold-companion loader header and field-order-only sequencer
packing. Linker wrapper preserves the native APP limit and uses
the archived read-only paired layout. It is deliberately never-install.

| Model | Stock XIP overflow | Guarded XIP overflow | Guarded RAM reserve shortfall |
| --- | ---: | ---: | ---: |
| Elite | 133,716 B | 35,536 B | 2,576 B |
| Pro | 133,704 B | 35,528 B | 2,576 B |

APP delta relative to stock is 98,180/98,176 B. The exact guarded maps
contain .pool 0x52a10 against total 0x54000 with mandatory 8,192 B reserve.
The linker stopped at the real XIP boundary; there is no qualifying image,
tagged companion output, migration evidence, or permission to consume
a live user slot. No runtime tests. This is not the earlier historical
v4 success or the narrow Synth One-only DNR3 diagnostic.

To replay: copy the combined-source folder to guarded-fit/tree. Apply
combined-slot3-fit/slot3-fit-guard.patch and composed-controls/fittab-guard.patch
from the restored handoff in that order. Copy the matching historical
workspace-recovery/fit/exact-extra/firmware/src/app/cold_companion_lab.h.
Run the composed-controls/compact-sequencer-layout.py transform once in
that tree. Use the recipe and same private toolchain inputs. Store D9_LAB_OUT
inside the isolated tree (the compiler's workspace expects relative paths).
