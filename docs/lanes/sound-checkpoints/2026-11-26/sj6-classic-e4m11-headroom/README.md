# Same-size headroom for the short SuperJ6 Classic history

The fixed-source gain correction still does not belong in production. The
legacy six-voice reduction and centered half-scaling occur in the compact
oscillator mixer; simply undoing them can clip the downstream short
Classic I Q15 history. This standalone candidate is one possible way to
remove that specific ±1 choke point without changing gain, delay time
or ring bytes. It is independent of the long-delay bandwidth effort.

Use a signed 16-bit 4-exponent / 11-fraction format with native
integer-word round-to-nearest even, a .00390625 first normal, and a
2^-19 quiet grid. The top code is 127.96875. A real finite over-range
sample saturates, nonfinite data is mapped to signed zero, and original
zero/signed zero round-trips. Native arithmetic costs are **not** yet
measured (in particular the quiet-path shift/convert).

All 65,536 representable words, adjacent ties including the normal
transition, and 514,336 independent finite/exception/transition words
passed. On the same four generated Classic modulation fixtures, with
six-tone and coherent peaks crossing ±1, the 512-byte ring produces
−80.45 to −81.58 dBr wet error against unquantized float history.
The exponential low-level tail gave −75.64 dBr instead of stock
−47.29 dBr. These are synthetic heldout reference signals with the
identical tap timing and interpolation, **not** matched factory
render or full-effect target evidence. The stock low segment is not
bit-identical. Do not claim that it removes the disagreement in the
dry factory outputs, corrects long-delay aliasing, or passes native
memory/flash/cycle limits.

On Mac:
```sh
cc -O2 -ffp-contract=off probe.c -lm -o probe && ./probe
cc -O2 -shared -fPIC check.c -o check.dylib
python3 verify-word.py ./check.dylib
```
