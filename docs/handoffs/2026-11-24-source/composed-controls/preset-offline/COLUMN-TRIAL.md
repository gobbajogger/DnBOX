# Exact cross-bank transpose experiments (offline)

All runs compared against the authorized original exports using the pinned corpus loader: 640 SuperJ6 + 906 Synth One presets, 25 banks, 221,013 ordered native-ID/binary32 words. The reconstruction includes every exact word (including raw sign), program name UTF‑8 bytes, source position, bank identity/name and stable 64-bit bank/program IDs derived identically to the DNB2 reference. A decoded in-memory pack is compared to all source records and all columns must be fully consumed. No preset payloads or source archive bytes are written by these scripts. Nothing here is a device codec, installable pack, production format, or native XIP/RAM claim.

## Reproduce

Requirements: Python 3.12, NumPy, zstandard; built-in zlib/lzma. Retain the neighboring `shared_dict.py` and the native checkpoint helper files unpacked in `source/`. Provide two private local ZIP paths, in this order.

```sh
OPENBLAS_NUM_THREADS=1 python predict_exact.py "$J6_ZIP" "$S1_ZIP"
OPENBLAS_NUM_THREADS=1 python predict_exact.py "$J6_ZIP" "$S1_ZIP" cross
OPENBLAS_NUM_THREADS=1 python adaptive_words.py "$J6_ZIP" "$S1_ZIP"
```

- `predict_exact.py`: Transpose exact words by native numeric field. Compare original row order and nearest-neighbor order by patch-wide equality across the bank boundaries. Try exact unsigned word delta and XOR, a sorted palette with local rank delta, lossless ULP-gap GCD factoring, sparse inherited-value overrides, and in the cross variant exact sparse alternatives against an earlier equal-support sibling column with optional conditional discrete dictionary.
- `adaptive_words.py`: No stored field histograms. A deterministic Fenwick frequency model learns existing symbols from the stream and emits one escape per first-use word, addressed through one exact sorted 21,045-word cross-engine dictionary. The dictionary, first-use references and adaptive arithmetic payload are all included in the measured packed values. The sign and mantissa remain unquantized. The magic determines the final codec; each envelope includes its format, complete size/extents metadata, and a CRC. Full data are decoded from these in-memory envelopes, not the original lists.
- The `lzma` experiments use host LZMA2 (64 KiB dictionary) and the `zstd` experiments use host Zstandard. Native decoder size, available working memory and independent random-access/timing have not been qualified.

## Fully accounted aggregate

| Model / order | Metadata B | Values B | Header / CRC B | Total B | Tight 4 KiB allocation |
|---|---:|---:|---:|---:|---:|
| Adaptive column predictors, source order + LZMA | 30,649 | 156,227 | 29 | 186,905 | 184 KiB |
| Adaptive column predictors, graph order + LZMA | 31,491 | 145,623 | 29 | 177,143 | 176 KiB |
| Graph ordered + sibling conditioned predictor + LZMA | 31,491 | 142,038 | 29 | 173,558 | 172 KiB |
| Online adaptive arithmetic, graph order + Zstandard | 32,467 | 138,984 | 30 | 171,481 | 168 KiB |
| Online adaptive arithmetic, graph order + LZMA | 31,491 | 136,152 | 30 | **167,673** | **164 KiB** |

Tight allocation rounds the entire global stream once. It excludes the pool's separate 8 KiB redundant directory reservation (the advertised 112 KiB is already data capacity), any BREAK, Korg data, and flash transaction spare space. The tight best still needs 41 sectors, 13 more than available. It is not independently deletable/mutable by bank and it cannot establish a device fit. For comparison, our prior frozen-model prototype independently decoded/replaced/removed one bank while preserving other extents; it needed 59 sectors (236 KiB) for model plus banks. The original DNB2 baseline with 25 separate extents uses 71 sectors (284 KiB). Thus seekability and independently replaceable bank extent overhead must not be ignored when evaluating a codec.
