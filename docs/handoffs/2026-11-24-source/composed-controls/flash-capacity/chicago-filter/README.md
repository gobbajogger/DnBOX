# Exact Chicago coefficient lookup: storage and timing checkpoint

This is an offline experiment, not a firmware build, installer layout or release.
No private input arrays or emitted tables are included. All 16,385 A and 16,385 E
binary32 words round-trip exactly (32,770 comparisons), independently in Python and C.
The input is the exact original coefficient header from the authorized project sources.

The current compact baseline uses 27,740 bytes (including both offset tables).
The candidate uses 6,099 B for A and 15,629 B for E in 128-entry segments,
including offsets, residuals, and all per-segment headers, saving **6,012 B**.
Each value is predicted with at most three signed 32-bit fixed-point products,
then reconstructed with a stored exact correction. Offline encoding checks all
intermediate bounds; no precision reduction. The host C test did not use the
firmware's repeated-index cache; it compares uncached lookup. In three adjacent
2M-lookup runs on the x86 host, the baseline took 301–321 ms, the candidate
164–187 ms, with identical checksums. That is not a PI32 timing claim.

The current diagnostic distributes the two table streams and indexes between
Slot 3 and FITTAB. These byte savings could open a sector in that diagnostic
layout but do not restore the user-owned Slot 3 or qualify a storage migration.
Do not count the same sector as both asset space and preset space.

The separate 909 clap tight-loop change was host bit-exact and linked both
models at the previous footprint but failed the strict gate: Elite offsets
0/1 at 12,455/12,445 us; Pro at 12,456/12,463 us. All had three dropouts.
Do not promote it. Another sampled exact preset codec (native grid encoding)
used 207,360 bytes before directory/reserve, larger than the current
176 KiB co-packed bank baseline. No codec or layout change grants a release gate.

## Reproduce

Requires Python 3 with NumPy, and a host C compiler. Set SOURCE to the
authorized Chicago private header and BASEGEN to the pinned project's
`tools/gen_chicago_compact.py`. Run from this directory. The emitted arrays
are transient private test fixtures and must not be committed.

```sh
tmp=$(mktemp -d)
touch "$tmp/lossless_table.h"
python "$BASEGEN" "$SOURCE" "$tmp/original_compact.h"
OPENBLAS_NUM_THREADS=1 python filter_words.py "$SOURCE" --fast32 --emit-test "$tmp"
cc -O3 -Wall -Wextra -I "$tmp" filter_speed.c -o "$tmp/filter-speed"
"$tmp/filter-speed"
rm -f "$tmp/original_compact.h" "$tmp/p_a.h" "$tmp/p_e.h" "$tmp/lossless_table.h" "$tmp/filter-speed"
rmdir "$tmp"
```
