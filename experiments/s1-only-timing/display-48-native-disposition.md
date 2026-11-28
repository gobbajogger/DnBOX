# 48-row display native disposition

Both Elite and Pro keep **69 rows** for now. No production or installer
changed. This checkpoint preserves source and the measured blocker for
the complete run, not a claim of 10,080 B reclaimed. See
`large-ram-cuts-checkpoint.md` for test invocation and layering.
The ordinary-knob patch in this commit supersedes the earlier initial
arm with a corrected anti-aliased compressor pixel on the row-48 seam.
True-page host runs: **912** exact raw RGB565 footers with and without
input/theme changes at the first transfer. 270 LIST input mutations
remain exact at the independent actual-48 test, 19 SMC captures and
84 cold-boot cases passed their comparisons. A separate actual
extracted Clip Keys painter passed 64 mutation cases.

Use `main-remaining-fallback.patch` after the previously staged main,
Clip and ordinary patches. That confines the remaining fixed 69-row
native route to the legacy bass matrix. Ordinary route PNG pairs still
matched with the reduced fallback (139). The cold sizing patch applies
to editor and project maintenance code, not the audio render path. With
it, the intermediate originally linked 452 B free Elite and 460 B
Pro; canvas still 69, no claim of new pool bytes.

`live-display-48-legacy-tape-lab.patch` is a rejected experiment to
use 32 scratch raster rows with a compact display command tape for the
legacy matrix. It captures state before input is pumped; there is no
borrowing of main stack for tile data. Actual legacy host pixels matched
in 420 scenarios; observed worst tape occupancy 6,007 B against
7,680 B available at 48 rows. This is not a complete overflow bound,
so 69 rows must remain in this experiment for the existing fallback.

Even on a special size-only arm without the built-in scripted sound
and note test (apply `strip-display-smoke-fixture.patch` before the
legacy tape overlay), the corrected recorder exceeded app flash by
**1,136 B Elite / 1,124 B Pro**. Neither linked; there are no release or
audio timing numbers. The size-only arm does not measure the full
stress scenario. Do not enable this recorder just because it passed
host pixels. The next credible large cut is to extend the existing
Synth One matrix telemetry to capture only Chicago/Miami graph data
and reuse the painter, rather than recording every graphics primitive.
