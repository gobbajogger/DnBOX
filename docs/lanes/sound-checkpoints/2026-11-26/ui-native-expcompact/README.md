# Exact APP savings for guarded Synth One and 909 transaction

Two local changes at the exact retained guarded donor:

1. EXP controls: every original binary32 correction remains. Unused high
   planes are omitted per row (2, 3 or 4 bits), and one immutable
   16-bit descriptor holds offset in 16-byte units, width and minimum.
   Decoder masks exact bits and leaves every original Float32 predictor
   operation unchanged. One padding byte permits its fixed two-byte
   fetch even at the final row. This is a changed lookup cost, not
   a runtime performance claim. No approximation, decoder length
   scanning, shared state, or new RAM storage.
2. Keep the 32 odd reciprocal entries of the 64-slot voice-span fixed
   array. Binary exponent subtraction restores the exact even values
   by a power-of-two scale. The usual 64-frame span has a direct
   1/64 path, so no new loop on that common case. Other admitted
   even values may take up to 5 shifts; out-of-table division is
   unchanged. No new global state.

## Native guarded result

| Source arm | Elite APP free | Pro APP free | Usable pool |
|---|---:|---:|---:|
| Original retained worker | 280 B | 292 B | 564 B |
| Odd-only measured | 376 B | 388 B | 564 B |
| Both changes on retained worker | 484 B | 496 B | 564 B |
| Both plus independent 909 txn on coldread donor | 100 B | 112 B | 564 B |

Combined +204 B on plain retained worker, actual 909 composition now links
on both. Existing guarded layout/flags retained; FDATA/FITTAB/slot3
diagnostic spans unchanged (not claiming newly allowed regions). Separate
per-model native receipts verify ELF and table symbols; no device image
was installed; these guarded images are not production artifacts.

Raw EXP expected result: 2,432/2,432 exact words for each generated model
against the pre-existing independent frozen oracle, repeated on composition.
Exhaustive former inverse table: 64/64, full tester actually 131,075
inclusive original fallback and boundary values. Regenerated model
x0x_drum_samples.h byte-identical to original on both models. Host proof
does not replace loaded native real-time/audio qualification.

## Reuse

- Apply exact-app-fit.patch to a verified isolated source tree
  (source-base.sha256 records exact donor inputs). The same two source files
  were byte-identical on the independent 909 transaction's coldread donor.
- The existing guarded link wrapper skips generate() by design. Explicitly
  execute `python tools/gen_drum_samples.py build/smk37-elite/gen/x0x_drum_samples.h`
  and repeat for `build/smk37/gen/` before invoking unchanged guarded link.
  Do not label copied, non-regenerated headers as this EXP build.
- Run `bash ui-native-expcompact/exact.sh PATH_TO_ISOLATED_TREE` after
  regeneration. The immutable combined ELF targets in native.receipt are
  in the sibling ui-native-expcompact-txn tree for the timing owner.
  Compared all four transaction source files byte-for-byte to the timing
  lane, including its exact physical 220-sample cap.

Also linked the timing owner's matched coldread + compression **without**
transaction separately: Elite/Pro +548/+560 APP, both +564 usable pool.
Native receipt contains both immutable ELF identities to isolate the
transaction from changed table access cost in loaded A/B.
