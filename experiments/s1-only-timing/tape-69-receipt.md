# 69-row prerequisite: immutable display tapes

This patch keeps ordinary raw 69-row MAIN rendering and the existing global
canvas size. It relocates LIST and SMC tapes after a 48-row RGB565 raster and
replays each main band as 48+21. The existing 16-row tile hash indices and
physical boundaries are unchanged. LIST's owned footer uses 48+34 so it never
writes through its own tape; S1/drums retain 64+18. SMC needs 28,560 B and LIST
28,072 B. Both capacity assertions use the actual canvas sizeof; full-length
footers and boot still need conversion for a real 69-row build.

- Native Elite at 82 rows links; 3,140 to 2,916 B XIP free (+224 B image);
  unchanged 27,020 B pool margin and 96,036 B ordinary RAM use.
- 139 ordinary host screenshots match pixel for pixel, 96 distinct.
- At actual 69-row canvas size (33,120 B), 270 LIST render pairs match with
  view, palette and LIST contents mutated after window one, spanning 6 themes,
  5 parts and 9 scroll positions; MAIN/FOOTER invalidations remain set.
  Focused fixture only: the global ordinary footer assertion is temporarily
  disabled for this test and no unconverted 82-row painter is called.
- Prior SMC flow passed with 19 pixel-identical screenshots. Cold/dirty refresh
  and native timing still need the combined test.

The broader MAIN conversion is separate: useful for the 64-row follow-up,
but legacy bass/PERF fallback still uses the full 69-row window.
