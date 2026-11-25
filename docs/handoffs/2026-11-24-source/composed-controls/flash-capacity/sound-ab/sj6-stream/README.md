# SuperJ6 causal 4-bit feedback experiment

This is a new **host witness and isolated PI32v2 object**, not a firmware patch.
It replaces the offline ADPCM/zero-phase witness with an actual causal
processing loop: independent stereo IMA predictor states, 96-frame restart
blocks, polyphase FIR decay input, six-phase interpolation, and .62
feedback through the coded history on every repeat. The reference is the
previous Build9-validated full-rate host kernel, in dual-mono mode.

At the same 496 ms delay, the old firmware loses 22.91 dB on the first
3 kHz echo. The bounded causal prototype loses 4.11 dB on repeat 1 and
15.67 dB on repeat 4. The independent bright stereo chord trails are
within 0.76 dB in overall level, but the residual and 5.05 kHz alias probe
show why that is not enough to sign off. A 49-tap filter, tested
before the final memory cut, was brighter but did not fit this state budget.
This fixed-stride prototype rejects one-second calls, so it doesn't replace
the original delay range.

Variable callback partitions are bit-identical, including two bounded
retimes, ring wrap and counter rebase. The explicit retimes require up to
103 decoded stereo history frames in a single raw-frame step. Unbounded LFO
edit cost, alias suppression, native cycle time, another engine sharing
the pool, and the full complete engine remain unqualified.

The real JieLi compiler produces a 4,092-byte state of which 3,978 bytes
are packed audio and restart headers. The 192-byte interpolation table is
read-only on the target. The separate input FIR coefficient table needs
38 bytes of read-only storage in addition to the object; call stack and
firmware integration costs are not counted. This is not evidence the whole
APP or a two-engine build fits.

Run from this directory with Python, numpy, SciPy and a C compiler:
```sh
python verify_stream.py --cutoff 3900
python generate_interp.py | cmp - interp-table.inc
"$JIELI/common/bin/clang" -target pi32v2 -mcpu=r3 -Os \
  -DSTREAM_NATIVE_LAYOUT_ONLY -c stream_probe.c -o stream.o
"$JIELI/pi32v2/bin/nm" -S --size-sort stream.o
"$JIELI/pi32v2/bin/objdump" -h stream.o
```
Put the prior checkpoint's `sj6_stereo_delay.c` and
`sj6_stereo_delay.py` in this directory for the host comparator.
