# Exact dual oscillator staging plus three-tom schedule

Source-only diagnostic. Same pinned strict 8-second Elite and Pro emulator scene, unmodified controller, fresh virtual NOR, six dry Synth One voices, 128 held halves and 132 genuine hits from each of the 808 and 909 kits. Zero skipped cycles and zero oversized display requests. This is not a normal-scheduler, wet, hardware, installer, or release result.

| Uninstrumented timing | Elite dropouts | Pro dropouts | Elite worst held | Pro worst held |
| --- | ---: | ---: | ---: | ---: |
| 402bd0e4 composition | 23 | 23 | 12,647.75 µs | 12,700.29 µs |
| Exact cold flash changes + in-place stereo, with two-tom prefix | 23 | 25 | 12,736.00 µs | 12,686.71 µs |
| Add raw four-frame dual oscillator staging | 16 | 16 | 12,359.04 µs | 12,322.29 µs |
| Also move the third 808 tom into the contiguous prefix | **10** | **10** | **11,876.63 µs** | **11,899.29 µs** |

The paired three-tom result is 771/801 µs faster than the 402bd0e4 held baseline, and 482/423 µs faster than the otherwise identical two-tom candidate. On both models 9/128 held halves still miss the 11,609.98 µs physical period, and the worst misses it by 267/289 µs (Elite/Pro). Both still miss the engineering target of 10,809 µs.

[Elite timing](owner48-zero-tom3-elite.json), [Pro timing](owner48-zero-tom3-pro.json). Separate native correctness builds yielded **0/160 different ordered pre-DMA audio blocks** against 402bd0e4 on [Elite](owner48-zero-tom3-fp-elite.txt) and [Pro](owner48-zero-tom3-fp-pro.txt). Those runs are not timing results. The timed builds have [1,156 B Elite](owner48-zero-tom3-fit-elite.txt) and [1,164 B Pro](owner48-zero-tom3-fit-pro.txt) free in app XIP; both preserve the 10,080 B display RAM saving and 43,316 B audio-pool margin beyond the unchanged 8,192 B reserve.

## Reproduction and boundaries

From a repository with both pinned donor objects, run `sh owner48-zero-compose.sh REPOSITORY NEW_SCRATCH_DIR`. Then run `owner48-build.sh` and `owner48-replay.sh` as documented in the [earlier paired result](owner48-release-overlap-result.md). The composed source matched the independent three-tom build tree byte-for-byte excluding generated outputs and receipt directories. Apply `render-fingerprints.patch` only for the separate correctness run; compare using `compare-six-loaded-dma.py`.

The shared waveform producer stages both non-bandlimited pulse/saw shapes in a 192 B local tape, without committing phase in advance or changing filter/mix summation. Admission requires stationary controls and warmed cached pitch steps. `bash owner48-zero-run.sh BASE_SOURCE` replays the standalone host check *before* applying the oscillator patch: 396,720 stereo frames, 6,120 spans, 72 mono changes and 919 admitted tiles matched full engine/arena states.

The three-tom prefix still uses only the existing stress-fixture reverse path. It preserves drum summation order: the two cores split at the next boundary rather than recombining separately accumulated mixes. Do not use the lab's selection rule to replace the production scheduler. The earlier diagnostic has not established general worker fault/tail behavior.

The [two-tom attribution](owner48-zero-dual-trace-elite.json) exposed how staging freed time on the synth core before drum completion. The [three-tom attribution](owner48-zero-tom3-trace-elite.json) shows frame 129,024 remains costly, with 5.19 ms 909, 4.20 ms remaining 808 and 0.96 ms final processing. On the other core, the 808 prefix is 3.09 ms and active Synth One about 6.00 ms after subtracting its drum-stage wait. Trace binaries alter layout, are below the app flash floor, and are not pass results.

Other measured options: moving rim/claves as well was slower (14 XRUN, 12,076.75 µs on Elite); moving the third tom only near its trigger was slower (15 XRUN, 12,066.58 µs). Moving the oscillator producer into existing executable SRAM saved no worst-case time, and the LFO-span alternative linked below the 1,024 B XIP floor. Early PCM predecode for all sampled cymbals used 36 KiB persistent RAM for only a small standalone timing change, so it is not in the paired result.
