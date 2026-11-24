# Shared preset and BREAK pool — experimental, not enabled

DNL2 stores exact ordered binary32 parameter words, stable eight-byte preset IDs and names in independently installable banks. The 46-byte little-endian header points to a bank name, 32-word delta-coded global dictionary blocks, per-field palettes, shared ordered layouts and indexed program records. The offline encoder accepts already mapped engine-native field IDs. It preserves repeated ordered writes and does not quantize. The current schemas are Chicago, Miami, Synth One and SuperJ6; drum and BREAK control schemas remain to be mapped. The website must preserve persistent IDs across filtering and editing, and detect any identity collision before installation.

DBD3 uses the existing 120 KiB slot-2 range only: 28 data sectors and two alternating metadata sectors. BREAK data occupies a reserved contiguous prefix; banks can occupy mapped sectors outside it. Bank replacement and removal require a caller authorization callback, and sample reservation and release require a verified sample-extent callback. No physical flash binding is supplied. Do not enable formatting, widen flash permissions or change the updater based on this experiment.

Run from this directory:
```sh
python3 test_codec.py
python3 test_pool.py
```
The tests use synthetic records and simulated NOR, without any private factory data. The Python encoder is an offline reference, not yet a browser compiler. The wire dispatcher is separate from production USB ingress.

Before product admission: implement actual saved-project reference checks, persistent ID migration, a browser DNL2 compiler and USB maintenance session, real FSMP reservation/recovery, installer preservation and migration from S1PK/legacy slot2. Stage decoding outside the audio callback. Prove Elite/Pro fit and timing at the current source, then exercise sample and preset recall and interrupted updates. The prototype is deliberately outside the production build. No device writes or firmware release.

## External Chicago/Miami factory experiment

`make_variant.py` generates separate copies of the two original control-mapping units with external DNL2 reads and a base-only dictionary (227 to 51 exact binary32 words). The original control-mapping and engine sources are not edited. The bridge binds complete immutable banks, requires their original numeric preset positions, and rejects missing banks without publishing a partial program. Its name buffers are transient. A production caller must manage their lifetime, quiescent binding and dependency checks before allowing mutations. It does not yet support sparse Korg bank recall, deletion of the legacy factory ID in use, boot fallback or UI enumeration of installed presets.

Run from this directory:
```sh
python3 test_codec.py
python3 test_pool.py
python3 test_korg.py ../original_synth
```

The last test compiles the existing factory APIs as an independent oracle and compares all 111 original preset names and full program snapshots, 2,322 macro words, 12,747 native words and 81 Chicago routing snapshots. The two DNL2 packs need 4,186 and 1,774 bytes (three 4 KiB sectors). The dictionary-validation refactor uses one checked block reader for admission and random access. The sample-only migration entry point refuses non-erased legacy capacity, S1PK, oversized or uncertain samples, and unknown directory bytes. It supports retry of a torn first directory initialization. The real FSMP/shadow validator must still be wired to the callback.

Native footprint experiment: the restored source is f1786ebec2616477163b62b029c7e77405a9c3de; the integration tip at measurement is 73749f55d17a0643f1ddd072114172d29eecc2ae. The four Korg source/metadata Git blobs are unchanged between them. The pinned PI32v2 compiler is 20260730.1, clang 4.0.1. The exact-order control links reproduced the restored Elite and Pro app sizes (551,444 and 551,436 bytes). The diagnostic external-bank link retains the pool/decoder/dispatcher and sample-adoption APIs, but no boot binding, production USB route, updater changes or saved-project adapter. Both physically link, leaving 976/984 bytes XIP, 9,024 bytes free pool, and 97,268 bytes ordinary RAM used. The 1,024-byte XIP reserve therefore still fails by 48/40 bytes before production glue. This is not current-tip firmware qualification or an installable candidate. Do not package or flash it.

The old installed S1PK pack overlaps the new directory sectors. The migration routine deliberately refuses it: a verified host backup, exact legacy-index migration, safe reinstallation and updater preservation are still required. The complete private synth corpus remains off Git. No release, protected flash change or device write.
