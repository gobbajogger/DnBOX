# Authenticated 906 Synth One upload: detached native-codec host witness

Exact firmware manifest f7d17a572dc1affdde651cb8764857da92ddb6308a3194d2bb72618ecbf98c12. Scoped to authentic ordered control records through the unchanged compact S1 renderer, not a firmware migration, asset package, bank UI, sequencer/ARP timing run, original-engine A/B, or installation.

- Authenticate the 906-upload ZIP and approved compression lab bytes, use the known original ordered-parameter compiler mapping, and corroborate all data through the real C S1PK decoder. All 906 identities in 13 banks retained by source bank + serialized array index + authored position + UID (UID is not identity). Repeated content entries and 11 repeated-UID groups with different content were not coalesced. S1PK carries bank table/program order/name and bit-exact parameters, but not archive UID/array-index metadata; those remain separate, held privately in the oracle and re-derivable from authentic input. No private identities, filenames, values, samples, or blobs in this packet.
- Existing 694 source pack stays untouched and independently loads its old ordinals. One exact content is absent from the upload at **existing one-based runtime ordinal 44**; it is retained as **last detached row, zero-based index 906**, outside the 906 uploaded identities. The old→detached same-content oracle is host-test-only, not included in the bank and not an ordinal migration, reopen rule, persisted remap, or per-UID alias.
- All 130,608 emitted ordered ID/binary32 pairs across 907 detached rows compared exact to the private source oracle. The native decoder's exact return count and selected program both checked before audio, so a failed decode or INIT fallback would fail.
- Every detached row genuinely rendered through two independent actual compact processor objects with separate aligned FX arenas. Uniform boundary schedule: 44.1 kHz, 128-frame blocks, MIDI 57 vel99 at frame 0, MIDI 64 vel81 at frame 2048, release 64 at 4096 and 57 at 4608, total 8192 stereo frames per owner. All 907 were audible and finite, the independently rendered owners matched exactly, and no PCM was persisted. All 694 original ordinal renders on both owners matched their detached content counterpart by full float stereo byte comparison. The schedule targets compact owner processing, not shared arp note ingress or hardware timing.

## Storage and runtime limits

| Measure | Verified |
|---|---:|
| Existing 694 S1PK | 61,208 B |
| Detached 906 + legacy-only S1PK, exact codec | 85,500 B |
| Existing assigned FL_FDATA | 61,440 B (60 KiB) |
| Existing slack | 232 B |
| Added pack bytes | 24,292 B |
| Excess over existing allocation | 24,060 B (6 sectors / 24,576 B if this representation is kept) |
| Detached program records / global values / column dictionaries | 58,355 / 19,542 / 6,083 B |
| Header / bank table / index table | 32 / 265 / 471 B |
| Other packed section counts | See aggregate codec log, including floor-to-byte bitstream accounting |
| Current host compact synth object / FX arena, per owner | 3,440 B / 4,252 B, same for old and detached packs |

This is the **measured existing representation**, not a lower bound for lossless compression, a proposed flash change, or a native-fit result. The detached runtime can decode these rows in the unchanged native-style loader without per-program RAM growth; scratch input/check/PCM arrays are host-only and not product storage. 907 distinct playable positions plus INIT require at least 10 selector bits (512 insufficient); current [7]/[21] fields can carry that numeric width, but production validator/selector caps it at 694 and a number alone cannot identify the catalog revision. The measured pack growth excludes new persistent identity metadata, correct prior-format admission, transaction/recovery metadata and bank installation/migration. Direct numeric reindexing is unsafe and forbidden by this analysis.

Reproduce from exact source and supplied private files: `./replay.sh EXACT_SOURCE UPLOADED_ARCHIVE COMPRESSION_LAB_ARCHIVE LEGACY_S1_PACK`. Wrapper uses mode-0700 scratch and deletes only its own transient output; prints aggregates only. All generated pack/ordered-word oracle products remain private, no source mutation, native tools, Git or hardware writes.
