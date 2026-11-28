# Keep glide history out of LCD stripe replay

Apply `glide-advance-outline.patch` to the regular 69-row paired
diagnostic. The only live caller remains the existing legacy glyph.
A future frozen legacy renderer should advance the fraction once, before
the first LCD input callback, and pass the resulting fraction to every
window. Only do this if the original visual path would render that
visible glyph. A target change across the seam belongs to the next
scene, and a same-target current-pitch advance must still move the dot
once that next scene is captured. Preserve separate histories per part.

Generate `glide-advance.inc` from the exact patched UI source: extract
`matrix_glide_history` through the close of `matrix_glide_advance`
(excluding the `matrix_glide[NBASS]` instance). Compile
`cc -std=c11 -O2 -Wall -Wextra check-glide-seam.c -o check-glide-seam`.
The host checks match all old history fields, fraction and integer
glide cursor for 200,000 updates plus a targeted modeled input seam:
outgoing frame still at .5, new target starts at zero, and subsequent
movement on the same target returns .5. Not an actual sliced-painter
pixel, boot, or deadline test.

Native Elite/Pro links: 1,156/1,164 B XIP free (+32 B vs donor),
95,408 of 98,304 ordinary bytes (+16), 33,260 B pool above the unchanged
reserve. This is isolated preparation only; it does not reduce RAM or
change the verified 69-row baseline.
