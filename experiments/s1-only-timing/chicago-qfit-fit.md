# Chicago FAST32: exact flash saving with unchanged decoder

The existing table compressor chose one maximum fixed-point coefficient
precision per polynomial degree. Searching smaller precisions offline
produces a shorter exact residual code for many segments. Keep the original
record as a fallback; a candidate can replace it only if shorter and no
larger on any of: polynomial degree, total coefficient varint bytes, rounding
branch presence, or residual bit width. For each lookup index these bound all
loop trips and variable-length reads. No renderer, decoder, sample, or
firmware memory layout change. The build generator verifies each decoded
uint32 word against the original 16,385-entry input arrays.

| Table | baseline | fixed-point search | bytes saved |
|---|---:|---:|---:|
| Chicago A | 5,583 | 4,854 | 729 |
| Chicago E | 15,113 | 14,480 | 633 |
| Total | 20,696 | 19,334 | 1,362 |

Actual partial 48-row diagnostic: **Elite 484 → 1,844 B free; Pro 1,856 B free**. Elite gains 1,360 B
after link alignment, unchanged 33,260 B pool margin after the 8 KiB
reserve, unchanged 95,424 B RAM .data+.bss. Runtime decoder and pair
lookup sizes and addresses unchanged in that paired native build.
No standalone native audio timing claim. No production use or push.

Source patch only touches tools/gen_chicago_fast32.py. Apply with patch
-p1 in the same diagnostic tree and run /tmp/dnbox-ramcut/build for Elite
and Pro. To compare emitted headers from the two links, run verify-c.sh
with source, original generated directory and patched generated directory;
then run python3 check-decoder-cost.py with those two generated directories
in the same order. Host C exercised both actual entry points and hot cache
hits, 245,775 exact float-word comparisons (all values, in three orders).

## Held follow-up, not part of patch
Offline minimax fit reaches 18,477 B (another 857 B), no additional decode
work versus stock. Relative to the smaller qfit variant, some individual
segments would use more work; enforcing no increase versus qfit yields
19,189 B (only another 145 B). Saved in minimax-experiment, not native linked
or recommended while qualification is running.
