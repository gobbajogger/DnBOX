# Full Part Sound and wet-render checkpoint

Development lab only. Do not promote to the installer. Build both native targets with `X0X_UI_SMALL_CANVAS=1 X0X_D9_TANH_BOOT=1 MODEL=elite|pro bash build.sh` and the normal JieLi dependencies.

The renderer snapshots Drum and Synth One Sound content across all three display strips. The Synth One test exercised eight combinations: both owners, both matrix windows, applied-patch release or not-ready-to-ready, with owner/page/palette/ruler changed between display uploads. Its single ALNK0 mask was checked by host instrumentation; native blackout and stack high-water are not yet qualified. All 139 broad host screenshots were byte-identical. The Drum Part precursor retains its 1,584 focused host comparisons.

The optional D9 table is losslessly decoded once into the audio pool before DMA; all 1,026 initialized IEEE words matched stock in all six native cold/boot/loaded RAM dumps. Compared with the unexpanded lab, it trades 4,104 B shared RAM for 3,160 B flash. The net shared RAM recovered versus the original canvas build remains 22,776 B and 27,392 B remains after the 8,192 B fixed reserve.

| Model | App flash free | Worst loaded block | Late / XRUN |
| --- | ---: | ---: | ---: |
| Elite | 732 B | 11,528 µs | 0 / 0 |
| Pro | 744 B | 11,491 µs | 0 / 0 |

Strict native emulator, 360 MHz, same 922-event tape, no skipped cycles or loaded NOR changes, Synth One 518 with arp, delay and Room on and Chicago on B. This specific replay has zero dropouts, but both exceed the 10,809 µs headroom target. Reference-mode packages on the diagnostic branch are not the stock build.

The optional S1 host fault test is generated into a **disposable copy**, not a shipping tree:
1. Run `python experiments/part-sound-qualification/s1-full-matrix-fixture.py "$PWD"` in the copy.
2. In its inherited `tests/host/smc_ui_test.c`, replace the unused 13-row title fixture's `cv_commit_rect(1, 0, MAIN_Y);` with `cv_commit_rows(1, 0, MAIN_Y, 13, main_tileh);` for small canvas.
3. `X0X_SYNTHONE_PATCH=1 X0X_HOST_DEFS='-DX0X_SYNTHONE_PATCH=1 -DX0X_UI_SMALL_CANVAS=1' SMC_UI_TEST_FILE=tests/host/ui_s1_full_matrix_fault_test.c sh tests/host/smc_ui_test.sh`.

Package SHA-256: Elite `d1a40f21c55690bbf34893031f9b667ab087694254f1c8ae3323f12686a45d19`; Pro `6fcf22d4e26073217e4f2d681da0d297b7e9ecbe03820bcfa265215618453177`.
