# Native immutable 909 relocation: latest wheel Init-only lab

Exact source-wheel-native-scale full ledger 8cf74b7a73a97873e136e0f0d738531c9a852a4e84eebe683793413bfd2bd2be,
plus prior init-only-legacy-root-prune patch. Prior read-only wrapper and
boot diagnostic exactly reused (init-fdata-readonly-lab). No live
modified source, package generation, physical device/flash, published
artifact, or copied owner records. This is not a release candidate.

## Measured linkage

| Model | Native APP use | APP free | Prior APP/pruned red | Net headroom | POOL free above 8192 reserve |
|---|---:|---:|---:|---:|---:|
| Elite | 529064 | 23608 | -31248 | +54856 | 540 |
| Pro   | 529052 | 23620 | -31236 | +54856 | 540 |

Identical fixed APP cap 552672; no PSRAM. The exact three ADPCM assets
(909 crash/ride/closed hat, 55,275 B total) are byte-compared from compiled
sections against native generated header values. A model-specific
32-byte guard joins them in a 55,307 B immutable envelope at physical
FDATA [0xE5000,0xF4000), 6,133 B not occupied by that envelope alone.
Body hash and each address/length in map.receipt. Slot geometry, project,
controller, boot and protected regions unchanged. Linker overhead relative
to previous same pruned failed-APP receipts is 419 B against the raw moved
bytes. Net numbers do not include optional guarded host-only control graphs.

## Executed guards and synthetic boot

Stock-app-style cold init checks exact paired header before dereferencing
body and checks body CRC. This is read-only compatibility/safety, not
cryptographic authorization. Guard-page policy and linked-native-body
tests cover erased/unknown/legacy/wrong pair and actual first, middle, last
body changes, plus short/long. Both model/link body bytes match; model
headers must differ and foreign headers fail.

Strict calibrated 360 MHz, 0 skipped cycles. The two matching synthetic
boots each play native 909 sequence/note stimulus, including the relocated
instruments: same 485,376 B nonzero PCM bytes (hashes in receipts), 236
halves with 0 late/xruns, Elite 3926 us worst / Pro 3941 us worst.
Normal synthetic project housekeeping writes appear only in sector
0xAF000; FDATA, XDATA, sample, and protected sectors are identical.
Both legacy-shaped S1PK synthetic occupied fixtures take the legacy safe
path: no audio, no scanner, zero simulated writes, complete flash equal.
There is no real stored S1PK content in the disposable synthetic legacy
fixtures (deterministic generated opaque bytes after signature).
The launcher uses pre-existing public 73749 boot templates read-only;
never emits an update package or executes an installer.
This is a bounded smoke, not populated heavy scene or full gate.

## Deployment boundary

Both published 73749 test packages already place actual S1PK (61,208 B)
in type103 FDATA (61,440 B). A unit with that/current opaque occupancy
cannot take this relocation safely: available 232 B cannot coexist with
55,307 B, and the correct native behavior is refusal/preservation. No
conclusions on every current unit, no premise that Slot3 is empty, no
relaxed root/expiry/backup policy. Moving these three items specifically
works at link/read-only boot; shipping a paired App+data update needs a
separately validated reversible admission, verified exact same-unit state,
and rollback contract. POOL headroom does not improve.

Replay: bash replay-link.sh from the packet; optional explicit
--run-synthetic as second argument after the workspace, only with sole
emulator slot. It materializes one local source copy then cleans binaries,
seeds and compiler outputs. Only source, digests, text receipts retained.

## Same-link strict heavy and 909-only diagnostic

Second native link hashes exactly match the smoke receipts. Frozen original
922-event 8 s deadline replay at calibrated 360 MHz, offset 0, kind-specific
max-Tune helper produces real project store objects with both arp ranges
and on switches inherited from the unchanged fixture. Adopted exact 2,712
Sound bytes compared at native RAM before each full run. Normal stock
Chicago/Miami is a different seeded workload, not causal relocation A/B.

| Workload, unchanged guarded firmware | Peak Elite / Pro µs | Raw late = XRUN Elite / Pro |
|---|---:|---:|
| Stock Chicago/Miami | 12394 / 12296 | 4 / 5 |
| S1/SuperJ6 + both arps + maximum 909 Tune | 27966 / 27684 | 92 / 92 |
| Same Pro, 909 part OFF only | — / 26445 | — / 111 |

Exact ON/OFF repeat matches Pro native executable, frozen scene, opening
Sound and full final Sound hash, no skip, while differing PCM. Single
diagnostic saved SET switch is its real production part-off bit 0 (and
required SET outer metadata), checked in native target ABI at offset 30
in both adoption and final heavy snapshots. Existing sequencer, physical
audition and MIDI ingress honor it. No baseline changes or deleted
events. Warm synthetic inputs byte-identical before/after both paired
runs. Relative 909-off gets 1239 us off peak, but adds executed halves
396→474; late fraction 23.23→23.42%, so no meaningful fix to the
~28 ms bound or evidence that drum triggering chiefly causes it.
No assertion of an optimization candidate or relocation causality.

One-shot test recipes: use replay-link.sh WORKSPACE --run-heavy, with
D9_RUN_909_PAIR=1 for exact additional Pro ON/OFF. Run with sole emulator
slot. Stock and heavy traces red; no further offsets claimed.
