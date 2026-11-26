# SuperJ6 Classic short history E4M11 guarded qualification (rejected as-is)
Canonical 703dd0da99b45e3826ce5a05030680dc2fcffd12,
history.h SHA c54a9f0718b7dfdb525de9ff81c8549fe8b04d2a7364403cf340e0a73cb390fa.
Same ring, actual existing write and two reads, original fractional interpolation
and scaling order. Depends only on the short Classic history, not long delay or
unison rings, and never modifies gain, master, center or modulation. Header
is included in juno6.c only to avoid duplicate firmware libc declarations.

Actual fixed-source typed compact chords in eight 46,080 sample drives peak
3.247–4.417. Host short Classic return versus F32 history gains from
stock −5.84…−11.02 dBr to −80.86…−81.37 dBr, and release from
stock −69.47…−76.76 to −80.70…−83.92 dBr. Synthetic plus 514,336 word
fixtures retained. This is not external factory sound parity, and low
history intentionally differs from Q15.

Guarded Elite and Pro diagnostic ELF XIP +2796/+2808 free (stock +2972/+2984),
same +564 pool, same owner/part sizes. At 360 MHz actual two-owner 256-frame
short-effect call 540.42→1629.45 µs steady and 534.32→1688.90 µs low fade,
both boards identical independently. Rewrites similarly +1.1ms. No observed
virtual NOR erases/programs. Actual PI32 code does call memcpy from decode
(see target-codec-calls.txt), so this specifically rejects the canonical C
carrier as compiled, not the mathematical format. Do not adopt. No full
scene, device operation or release.

Existing guarded harness source donor in donor.sha256. Replay native.sh
elite/pro 0/1; tee guarded linker output to model/arm log names in this
directory, then replay.sh elite/pro 0/1. The archive carries no DSP binaries,
signed packages, input or output audio, presets or firmware images.
