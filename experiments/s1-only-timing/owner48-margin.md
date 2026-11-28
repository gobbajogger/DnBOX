# Keep all 10,080 B of display RAM and restore the app-flash floor

Use the exact recipe in guarded48-fit.md, but replace
frozen48-semantic.patch with **owner-48-snapshot.patch**. This is still
a diagnostic; do not install the benchmark firmware. The guarded 48
fallback removal and dependency-free Chicago generator remain exact.

The packed-input scene no longer duplicates 22 descriptors or raw values:
both are recovered from the captured owner and existing packed record plus
the per-control preview mask. During the synchronous painter, owner access
is checked once on capture; all visible layout references must belong to that
voice. The global scoped pointer is cleared before DMA/foreground service.
No DSP, transfer, feature, delay or stack-reserve edits.

Both native linked 48 rows (23,040 B). Audio-pool margin remains
43,340 B above the unchanged reserve, +10,080 B vs 69 rows. App flash now
**1,088 B Elite / 1,096 B Pro**, above the 1,024 B objective; improvement
of 768 B on each compared to the first guarded 48 arm. The intermediate
compact arm without the owner validation dedup gave 992/1,000 B.

Host: 420 quiet + 420 mutated actual legacy MAIN/footer comparisons were
exact, including live selection, theme, old sound, held step and parameter
lock changes; host snapshot 348 B vs 544 B before. On the complete route
replay with actual 48 backing and fail-fast oversize, all 139 PNGs exactly
matched the original 69-row paired baseline. No physical hardware writes.

Strict loaded results (six actual held voices, both real drum kits,
zero skipped emulation cycles, zero oversized requests): **118 XRUNs
Elite and 120 Pro**, vs the 320/328 B guarded arm's **108 / 106**.
Held maxima 13,134.54 / 13,153.38 µs, still above the physical 11.61 ms
deadline. This meets the memory and flash sizing objectives, not playback
qualification. Keep it as an alternate, not a blanket replacement of
the lower-dropout diagnostic. Code sizes in major audio routines were
unchanged, but linked addresses moved; cache sensitivity is a possible
explanation to test, not a demonstrated cause.

The separate native legacy witness on both models observed nine Chicago
and ten Miami rendered frames at physical 48 rows, zero oversized requests.
To repeat use the same instrumented witness as guarded48-fit.md.
