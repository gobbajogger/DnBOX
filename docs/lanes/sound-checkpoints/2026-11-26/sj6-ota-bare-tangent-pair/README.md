# Prepared tangent two-clock schedule, bare PI32

Run bash sj6-ota-bare-tangent-pair/run.sh from the pinned workspace. Source archive is pinned to SHA 8f416adac4846a2be2f9fa71ba702ad688dfaf395cbe692359c7817712450b8e, and the extracted stock.c / pair.c are verified byte-for-byte. The unused host export is stripped; wrapper adds identical 512-frame work and timer. One alpha preparation per arm is timed. Both use prepared g=.17, no fused arithmetic, same independently pregenerated per-voice inputs, matching ARM-independent output checksums. TIMER4 24 MHz; no interrupt/DMA/Sub/mix or physical device testing.

Active voices | Prepared baseline | Two-clock | checksum (both)
---|---:|---:|---
1 | 20,255 | 12,800 | af7f2fb5
4 | 76,925 | 48,520 | 1472bdaa
6 | 114,685 | 72,354 | 7739089b

Exact prior prepared stock tangent baseline ticks and hashes at every count; zero skipped emulation cycles. Candidate reduces 36.8–36.9% ticks. At six voices 5.888 µs/frame vs 9.333 tangent. Prior separate compact stock OTA (a different, non-source-authentic filter) was 5.230 µs/frame; difference is 0.658 µs/frame or 337 µs over 512. Candidate relevant linked symbols 542 B vs 540 (406 caller +134 clock), bare only. This doesn't qualify the underlying tangent approximation against the real source, active controls, full renderer timing, target capacity, or a release.
