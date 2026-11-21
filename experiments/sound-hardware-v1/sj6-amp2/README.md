# Compact SuperJ6 runtime and factory-field replay

Four decoded SuperJ6 fields reach the second-layer amplitude envelope through the normal `original_runtime_param` and `juno6_set` path. Add `JUNO6_APP_PARAM_BASE` to a decoded app ID before using that entry point. Stock macro IDs 0–17 keep their existing meanings. The four qualified app IDs are 119 (attack), 120 (decay), 121 (sustain) and 122 (release). Each direct secondary write targets layer 2, including when the original AMP link is enabled. This patch implements direct writes. Import of all factory controls, linked primary-to-secondary callbacks and a full original-app audio comparison are open.

Host replay requires GCC, Python 3, the corresponding compact/combined candidate source with the patch applied, the stock generated Chicago header, and `J6presets.zip` (451,262 bytes, SHA-256 `ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8`). The checker verifies each of the twelve bank files, the pinned clean control catalog, all 640 bank/preset identity pairs and every serialized name and binary32 value. The original files are comparison inputs. The repository includes the checker and aggregate measurements.

```sh
python3 replay.py --repo /path/to/combined --banks-zip /path/to/J6presets.zip
python3 replay.py --ubsan --repo /path/to/combined --banks-zip /path/to/J6presets.zip
```

Pass `--gen /path/to/stock/generated/headers` if the matching headers are outside the tree. The host audio fixture pans the two active DnBOX layers apart, switches off the stock chorus, and compares the same note and release at raw knob values 0 and 0.75. Each comparison spans 32,768 stereo frames at 44.1 kHz. The peer is an independent sounding part. Physical scalar tolerance, fixed before the replay, is 0.00001 seconds or 0.00001 sustain fraction, following the archived AMP2 factory oracle.

| Factory treatment | Rows | Evidence |
|---|---:|---|
| Runtime AMP2 writes | 2,560 | Physical value checked against the binary64 transfer, raw checked bitwise, primary and all other secondary parameters unchanged. |
| Other stored app controls | 87,640 | Exact name-to-ID conversion and candidate raw-storage readback, including 151 recognized legacy entries. Their physical behavior remains unqualified. |
| Global key lock | 75 | Each program preserved the host fixture's externally owned values at IDs 212–214. |
| Named legacy fields without an app ID | 274 | Complete name and binary32 value round trip in the host decoder. No app ID or DSP effect was assigned. |
| **Total serialized entries** | **90,549** | All 218 slot presence/value results verified on each fresh part. |

All four controls changed the sounding second layer. Layer 1 produced zero changed samples. The peer remained byte-identical during the comparisons and sounded after them. Twenty-two programs requested decay above 16 seconds. The greatest absolute scalar error was 0.000002564. The common owner retained its 16-second default. Only a SuperJ6 part admits AMP decay and release to 24 seconds. Existing per-part state measured 2,264 bytes, the Juno synth 2,356 bytes.

With the original combined source, this replay compiles and the first tagged AMP2 runtime event returns a failure. Later observations use the new compact runtime and compare controls and candidate audio. They provide scoped S3/S5 evidence. N01 still requires complete original SuperJ6 app renders for 14 presets when the original 14/56 comparison archive is unavailable. The authenticated sampler-control and musical-loop archives contain component recordings and cannot qualify that gate.

[Factory comparison input](https://chatgpt.com/library?file_id=libfile_fde3_f-6ff4bb216f9c8191bb54bd7c912e4624_2021479872048191acbfdca1f870a539&account_id=personal) · [Earlier isolated AMP2 routing and full-duration checks](https://chatgpt.com/library?file_id=libfile_fde3_f-5cf570afc43c81919ef54276b16c2f52_2021479872048191acbfdca1f870a539&account_id=personal)

The source recipe `gzip -dc amp2.patch.gz | git apply -` applies on the compact candidate, with preimages in the [native packet](../stock-fit-20261121/native-receipts.tar.gz). The legacy packed-event negative and stock attack positive controls are under `entry/`.
