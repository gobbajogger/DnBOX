# Detached exact +213 Synth One overlay (host accounting only)

Hash-authenticated 61,208 B Build27 S1PK decoded by compiled product C reader and 906-row upload by audited lab loader. All 906 rows retain bank, archive-order bank sequence, original array index (implicit in sequence), signed position, case-exact UID and UTF-8 name. Payload reuse is not identity merging: 693 alias rows use the lowest matching old ordinal to fetch *words only*, while old 694 names/orders/ordinals remain unchanged and separately addressable. Old ordinal 44 is not in the uploaded catalog and survives in the untouched base. All 213 new rows are distinct, all 906 entries checked end-to-end with the same 144 ordered IDs and binary32 words. No raw payloads or name/UID output.

Each delta anchors once to an old ordinal, holds a 144-bit changed-field bitmap and records exact 32-bit XORs. Per-delta raw aggregate 37,476 B vs 122,688 B for independent raw words; nearest old anchors overwrite 2–76 words, mean 38.99. Catalog in independent 16-row bank chunks has 33,222 B including 496 B for the directories; every source UID genuinely is canonical uppercase UUID, reconstructed exactly from 16 bytes and tag, not re-identified. Separate **modeled** 88 B pair identity/version/CRC/length wrapper and 239 B bank metadata cost; record and catalog bodies were actually compressed/decompressed and compared, wrapper is not implemented firmware or an authenticated format.

| Delta read grouping | Exact raw-DEFLATE body + block directory | Overlay with catalog and wrapper | Combined with untouched stock base |
|---|---:|---:|---:|
| 1 new record | 34,018 B | 67,567 B | 128,775 B |
| 8 new records | 28,406 B | 61,955 B | 123,163 B |
| 16 new records | 26,837 B | 60,386 B | 121,594 B |
| All 213 in one block (no useful random seek) | 24,196 B | 57,745 B | 118,953 B |

A whole-catalog 27,529 B compressed-stream comparator trades away bounded bank lookup; joining this comparator to one whole delta stream yields 52,052 B before any catalog seek support (113,260 B with base). This is a measured option with explicitly weaker seek, not a lower bound. Already the bounded designs exceed the 232 B free in current S1PK region and exceed the separately reported ~85.5 kB replacement even before considering whether that replacement carries this exact alias/UID identity scope. The comparable increment is the overlay alone when stock is immutable, not +213 × 576 or a whole wholesale pack rewrite. No combined S1/SJ6 fit follows.

16-record witness materializes ≤3964 B of uncompressed delta block, plus 576 B anchor if fully copied. Catalog output block ≤833 B and may be decoded separately. DEFLATE window plus decoder state, executable code, native safety, latency, rights, versioning, atomic storage and POOL remain unmeasured; a host zlib proof is not deployable or independent native seek proof. No format/product/map/bank changes. Reproduce locally: python perf-final-proof/s1-detached-overlay/measure.py . ; only aggregate logs and source are preserved.
