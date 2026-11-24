# Legacy-first Synth One S1PK host experiment

Exact firmware manifest `f7d17a572dc1affdde651cb8764857da92ddb6308a3194d2bb72618ecbf98c12`. Same authenticated 906 ZIP, old S1PK, and approved compiler helper as the upload-first experiment. No production format, source/product writes, index migration, Git, private data publication, native/emulator/device smoke, or external writes.

## Identity and admission

- First 694 payload rows retain exact original one-based ordinal, exact name, and all 144 ordered binary32 commands; original 12 bank table rows also retained. Ordinal 44 is unchanged in its old position and is not in the uploaded picker. Append exactly 213 unique incoming payloads absent from the old corpus, in the order first encountered.
- Detached picker in ZIP directory order and each bank's exact serialized array order preserves **all 906 identity/label rows**. It maps 693 incoming rows to old same-content payloads and 213 to appended payloads. Among 38 ambiguous picker references, prefer an exact existing name then lower same-content old index as a deterministic data lookup only. This is not original identity equivalence, a migration or a persistent redirect. All 11 repeated-UID different-content groups remain distinct. Full bank + source array index + position + UID + label remains separate from content references.
- C S1PK readback preserved all 130,608 ordered ID/binary32 pairs across 907 rows, plus exact native names in the old prefix; Python verified all 906 strictly packed selector references against the authenticated source words. Host C rendered all 907 rows with detached provider, finite/nonzero and bit-identical between independently owned compact DSP instances, 8192 stereo frames each. All 694 old rows additionally matched at their **same** index, full stereo byte equality.
- Actual old-app project readback (486 checks) saved and mounted all six storage objects under old 694 source, then loaded on proposed combined data under unchanged app validator/select path. At each physical owner, old ordinal 44 and a genuine same-content/two-distinct-old-label pair retained numeric selection, exact factory names, every stored object byte with no writes on load and the complete compact mix PCM. No native promotion of the new 213 selector ordinals; original runtime cap is still 694.

## Storage with actual existing codec

| Representation | Bytes |
|---|---:|
| 694 current native S1PK | 61,208 |
| Actual legacy-first 907-payload S1PK | 85,496 |
| 906 fixed-width × 10-bit picker reference vector | 1,133 |
| Legacy-first plus picker, before its identity/alias metadata | **86,629** |
| Previous upload-first pack, no identity sidecar | 85,500 |
| Previous detached overlay comparison, before identity sidecar | 121,594 |
| Allowed current physical FDATA | 61,440 |

Raw legacy-first payload fusion saves only 4 B vs upload-first. Picker is worth 1,133 B so combined layout is 1,129 B larger than raw upload-first while preserving the old ordinal prefix; it is 34,965 B smaller than the detached-overlay comparison. Above physical FDATA by **25,189 B before identity metadata**, requiring seven further 4 KiB sectors if this exact representation were ever retained (not an authorization or suggestion to change map).

Illustrative cost of retaining metadata outside this payload vector, not a proposed MCU decoder: all 906 full raw UTF-8 label fields + NUL = 16,341 B, host zlib level 9 = 6,976 B. Separate portable JSON witness for full UID/label/bank/position/array = 74,274 B plain or 32,952 B host zlib. Repetition can compress but a vector alone cannot restore those labels/identities, nor support safe project catalog admission/deletion. No compressed metadata or payload is retained here. This experiment is not a lossless entropy bound and cannot claim native installability or timing.

Reproduce with `./replay.sh EXACT_SOURCE AUTHENTICATED_UPLOAD_ZIP COMPRESSION_LAB_ZIP LEGACY_S1_PACK`. Private codec products and six fixture objects are temporary, mode-0700 scratch; output is aggregate statistics only.
