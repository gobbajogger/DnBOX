# CLIP KEYS live footer with a lossless 63+19-row raster

Narrow target experiment; `clip-keys-sliced-footer.patch` does not change
global `CV_ROWS` or other firmware views. In the native small-canvas path,
capture settings, chord, phase and theme values before any LCD transfers.
Replay identical RGB565 painters into 63 and 19 rows and stream each directly.
63 is divisible by the existing three-row full-width DMA granularity (the
same three-row sequence and final two-row packet as 82 rows at once). Unchanged
signatures skip the work; hash invalidations during transfers are not overwritten.
No LCD DMA uses a full persistent framebuffer or new output encoding.

Verified: native diagnostic Elite links with unchanged pool margin 27,020 B;
its instrumented build is 128 B larger than the same parent build without
this patch. Existing clip graphics fixture (legacy full-strip vs factored
reference) produced 18 byte-identical raw RGB565 captures, and the new footer
oracle matched 64 full-strip vs sliced comparisons even when the palette,
settings and key state mutate immediately after the first strip. The host
fixture validates geometry and pixels, not native performance: do not lower
the global 82-row buffer until every other route and foreground/audio
timing are covered. In particular do not count any global RAM saving yet.
