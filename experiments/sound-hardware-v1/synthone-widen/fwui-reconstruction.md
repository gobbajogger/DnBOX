# Editable widening prototype

[fwui-reconstruction.patch](fwui-reconstruction.patch) adds factory ID 121 as a fourteenth per-part control under `X0X_SYNTHONE_PATCH=1` and `X0X_SYNTHONE_WIDEN=1`. It retains the original 13-slot snapshot ABI, adds a sized accessor, and smooths the width target at 44.1 kHz with the source parameter's 0.1-second half-time. A 13-slot project loads with a dry widening target. Width and preceding history belong to each selected part.

Before the Linux workspace disconnected, the prototype passed all 14 × 128 editor positions, existing project reloads, note locks and stale-revision tests. The real application fixture checked 502,016 part A frames and 483,584 part B frames against the original Soundpipe smoother and the delayed/mixed output.

The attached patch was reconstructed from the recorded edits. All 16 sets of hunks match the published source at `1075a1960c1a3e1f593b6a509a52d1262df34332`. The tested scratch tree had no retained digest for a byte-for-byte comparison. Its first opt-in native build failed an existing target guard on the widened edit mask. The reconstructed patch includes the revised mask check. Compilation, ABI retention, both native packages, and both physical Clip comparisons still need a fresh run of these exact bytes. The ordinary 13-control interface and the separate selected-ownership diagnostic are the verified branch implementations.

```sh
git apply experiments/sound-hardware-v1/synthone-widen/fwui-reconstruction.patch
sh tests/host/synthone_widen_test.sh /tmp/s1-widen-test
```
