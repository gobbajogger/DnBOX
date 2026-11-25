# Native combined Chicago table + scheduler + display isolation trial

This is an experimental, complete-controls Elite/Pro diagnostic, not an
installer, release gate, or production layout change. No private tables or
virtual flash artifacts are included.

Start with the verified all-controls-fit-ramp-fixedpoints source and its
board-specific generated inputs. Apply the FAST32 native patch and use its
paired JieLi wrapper, then apply the guarded single-Chicago Core1 patch and
the display isolation patch included here. The latter also applies to the
legacy/non-FAST32 source. Generate the exact coefficient bytes with
filter_words.py and `--fast32 --emit-test` into private scratch. Use the
existing pinned virtual overlay and strict 360 MHz no-skip replay. The emulator
and 922-event fixture respectively have SHA-256:
`fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017`,
`d08718b1521119b252b5cca0c5e1db6981ea25a5a0b31b6ead60dceccb3f6271`.

The original scheduler held Chicago on Core0. Enabling one Chicago on Core1
exposed an additional writer even with dual Chicago serialized: the main-loop
cutoff display also called the shared audio pair-cache writer. Under a legal
interleaving both setters can leave a mismatched A/E pair with a stable index.
The UI-only reader now looks up just the immutable A coefficient without
touching that cache. Host results verify all 16,385 A words bit-identical on
legacy, compact-native and FAST32 representations. The audio lookup remains
unchanged by the cache fix; two Chicago parts still stay on Core0.

| Event offset (ms) | Elite us | Pro us |
|---|---:|---:|
| 0 | 11,235 | 11,100 |
| 1 | 10,840 | 10,863 |
| 2 | 10,868 | 11,209 |
| 3 | 10,804 | 11,223 |
| 4 | 11,111 | 10,810 |
| 5 | 11,110 | 11,119 |

All 12 loaded rows produced active PCM with zero late blocks, zero XRUNs and
no non-silent DMA repeats. Cold boot and guards passed separately. The pair
fits with Elite 352 B and Pro 360 B APP headroom, each with 464 B above
the RAM reserve; 4,988 B remain in diagnostic Slot3. Native C1 stack used
3,348 of 4,352 B with the bottom 256 B guard clear in every loaded row.
Separate passive profiles show Chicago voice and backend code on Core1
for both models. Do not use profiled PCM in place of a strict run.

Exact ELF SHA-256:
- Elite `fd81f23be3efa8301ea10f40bbbf854d9d64d8df452261c6f3597fe005880483`
- Pro `97aadaee37ae1688368413ffc41b91da7cde45955c5b5ddf2d05e0fb742a8e91`

Five of twelve meet the target of below 10,909 us; the worst is 326 us
over. Moving all 808 work to Core0 before remote Chicago regressed and
introduced misses. A more responsive 1/16 planner hysteresis also introduced
misses; keep the current split and 1/8 threshold. Still need measured
critical-path costs, fixed-input and dual-Chicago playback qualification,
and a viable safe storage layout before promotion. No device work or
publication.
