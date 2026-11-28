# Lossless usable audio RAM on the Synth One diagnostic

Native same-source pair uses f99f3464 experimental overlays. Relocate one
1,552 B immutable BREAK snapshot bank and 726 B of keyboard bookkeeping and
foreground event buffers from the audio pool to ordinary .bss already owned by
the app. Append an input subsection to keep the existing ordinary object
placement and its one startup zeroing sweep. No DMA buffer, stack, boundary,
object format, buffer count, or musical behavior was changed. No production
enablement or hardware access.

| Both models | Original | Relocated |
| --- | ---: | ---: |
| Pool bytes free beyond existing 8 KiB reserve | 33,260 | 35,540 |
| Ordinary RAM free | 2,912 | 632 |
| Elite XIP free | 1,124 | 1,108 |
| Pro XIP free | 1,132 | 1,116 |

The 2,280 B usable pool gain is **reallocation, not a total physical RAM
reduction**. The original and candidate Elite symbol addresses match for proj,
project_stage, _guard0, _c1_end and both region starts; only the selected
objects plus layout of later pool data change. Both .bss and .pool are NOBITS.
The published BREAK pointers still select four distinct, stable snapshots.
The native-like bank path passed 2,214,169 host assertions. The 37-key exact
ownership/ordering suite passed 1,250 assertions on the source donor with
the same keyboard relocation. The existing raw transaction suite also passes.
No native BREAK-active timing claim.

## Why staging stays 36,420 bytes

The native ELF has one 36,420 B project_stage in the pool and one live proj
outside the pool. The first 40 B are settings; the remainder of project_stage
already aliases the byte-exact undo shadow. Import copies raw sequential bytes
into this same slab; previous offsets can be retried and must return byte-exact
answers before VALIDATE, even for invalid input. ABORT restores the undo shadow
from untouched live project; failed sample recovery can keep the transaction
locked. Export snapshots and serves random READ requests throughout its hold.
A smaller general compression buffer cannot represent all accepted raw sequences
under that retained retry contract. Using a smaller lossy or probabilistic
shadow would change undo/export admission. Audio workspaces and LCD rendering
are not automatically unused in all the same lifetimes. A substantially smaller
stage would require a format/protocol or lifetime/ownership redesign with
transaction, failure, and real-time qualification, not just dropping a copy.
