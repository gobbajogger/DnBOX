# Motion encoding + private UI waveform cache relocation: linked diagnostics

Actual both-model guarded native builds from exact serial INIT-only +112 UI batching stage, no ROM or device operations. Checked-in SMC source recovered from 73749f55d in isolated stage, and two verified private inputs recovered from retained copies (not included). Restored baseline reproduces both entire +112 ELF hashes and 73 object manifests. All builds used the committed build-storage-guard.py.

| Arm | Elite APP free | Pro APP free | ordinary gap | Raw POOL free | Beyond hard 8704 |
|---|---:|---:|---:|---:|---:|
| Exact UI+112 anchors | 872 | 884 | 156 | 8756 | 52 |
| Motion 0ed86c7d | 888 | 900 | 316 | 8756 | 52 |
| Motion + move private waveform cache | 888 | 900 | 156 | 8912 | 208 |

Motion-only BSS -160, APP +16 both boards; 71/73 isolated sequencer/engine/DSP objects identical, only motion and its engine consumer changed. Actual host differential matched 180000 actions and 218400 writes/restores, digest 2160a4f6d28dff59. Relocation removes only X0X_POOL on the private waveform cache. Cache isn't used by audio or DMA; UI cache update/search and invalidate preserve lifetime. BSS and POOL both independently zeroed; no alias or pool boundary change. Symbol moves from 0x01c3952c to 0x01c09ba0, size 156. Ordinary BSS adds 160 due alignment, restoring incumbent gap. All 73 DSP/engine/seq objects unchanged vs ramp, plus exact FDATA/FITTAB/MORPH/SLOT3/XDATA payload including zero pointer relocations. XPOOL and guard pins unchanged. No production or device result. Subsequent read-only strict emulator playback is documented below.

Replay sequentially on each model using storage-maintenance/build-storage-guard.py: set MOTION_ARM=baseline to run.sh, then default run.sh for ramp, finally run-relocation.sh. All archive files source or receipts only.

## Subsequent emulator replay

Exact retained movement plus cache-relocation ELFs:

- Elite: `d4de703b3f051d55596629f63fdfe83ea54c736621cf74c1389f61fd654f53d1`, 14,568 µs maximum and 77 late/dropouts.
- Pro: `332d74a1cb18b944b9d028675e274ab12b5de68834fbfd94f9b4129bd30b445e`, 14,356 µs maximum and 42 late/dropouts.

Both had precisely 319,488 audible frames, zero skipped cycles or damaged mailbox, and unchanged virtual NOR with zero erase/program. This strict loaded S1/Chicago scene has no active SuperJ6 attestation. Both fail the <=10,809 µs and zero-dropout gate; no release. Exact source/maps pack SHA-256 `0ef4917825284c640cf054936c63b4b61861cd294cec6e0c0cf63d675135f1f1`; replay source/receipts SHA-256 `c363adb42b8331dd688e07a5620c8049c8b3533fdc689361e543874baf0e5082`.
