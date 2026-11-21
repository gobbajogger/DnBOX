# SuperJ6 secondary amplitude controls through the stock audio block

[Download the source, replay commands and measured results](typed-entry.tar.gz). Run the commands below from the extracted packet.

The isolated compact SuperJ6 candidate accepts original secondary amplitude fields at an explicit, owner-scoped live input. Original binary32 values cross the block boundary intact. Eight owner/field comparisons each changed full-application stereo output. The second active slot's isolated audio stayed byte-identical in all eight comparisons.

## Application contract

`engine_superj6_edit(owner, field, unit)` accepts typed IDs 119–122 for attack, decay, sustain and release, for either currently active Juno slot. The main-loop entry checks the field, owner, project-hold state, engine kind, runtime status and finite normalized range. Valid values enter a four-float per-owner staging area under the existing audio interrupt lock. `engine_render` applies staged values after part preparation and before serial frame-zero commands and keys. This is a bounded control-state update. Repeated updates to one field before a block coalesce to the final value, and the four independent fields commit in ADSR order. Zero-length/held render calls retain pending state. Cold selection clears that owner's staged and runtime state with the owner object.

The entry is intended for a future original-data importer. This candidate does not assign MIDI controls, create new UI controls, alter the legacy command queue, or persist the new values in the existing project cells. Live typed input is an additional application entry. The four stock Juno patches remain the product's published selector entries. S3, S4, S5 and S6 remain open for the full required feature sets.

## Measurements

Our full-application fixture selected two real slots through the published selector, used ordinary queued note-on/off and measured 32,768 stereo frames at 44,100 Hz per run. For each row, the fixture supplied raw values 0.0312500037 and 0.750000060 to the specified field. The left and right channels were measured after the application mix and master. A second run sounded only the other slot to check its independence. Every low/high peer pair had 65,536 byte-identical sample words and nonzero audio. A same-boundary low→high overwrite matched the high-only audio exactly. Untyped legacy calls on the previous candidate are documented in the sibling ingress review.

| Field | Owner A changed words | First changed frame | Owner B changed words | First changed frame |
|---|---:|---:|---:|---:|
| 119, attack | 35,001 | 64 | 51,145 | 64 |
| 120, decay | 34,930 | 168 | 51,337 | 168 |
| 121, sustain | 34,981 | 168 | 51,378 | 168 |
| 122, release | 32,638 | 16,448 | 16,256 | 24,640 |

The original factory generator supplied the four unrounded values for every one of 640 programs in 12 banks. The test staged two different programs together before each of 320 calls to the real one-frame engine render. All 2,560 observed post-boundary DSP raw fields matched the corresponding source binary32 bytes. The test-only linker wrapper observes the actual runtime call and immediately calls the real function. The candidate introduces no diagnostic hook. Invalid types and NaN left both complete owner structures byte-identical, failed submissions matched control audio, zero-frame calls made no setter calls, project bytes stayed unchanged, and cold reselect expired the selected owner's values while its peer retained its release.

Only the four AMP2 fields of each original program were sent. These tests do not admit the other original fields or establish full factory graph fidelity. The complete field name/value fingerprint (sorted by original name, SHA-256 over name plus little-endian binary32 value) is unique for all 640 programs. Original program UID is also unique across this corpus. Display names give 638 unique values. The four AMP2 values give only 539 unique tuples. There are 158 programs in 57 shared AMP2 tuples, with as many as 25 programs sharing a tuple. The stored MIDI bank-change/program-change number pair gives just 12 unique combinations in these files. Use the original bank and preset UIDs for a future importer.

## Stock footprint with the entry retained

The parent is the compact AMP2 + range dictionary composition `18f29e2ce3d83d034a76c6ca6ae79962c12d2350`. The isolated source changes are in `typed-edits.patch`. A separate measurement-only linker change adds `-u engine_superj6_edit`: the future importer has no production caller yet, so linker garbage collection must not hide its code or conditional consumption in sizing. Both final ELF files retain the named 130-byte API. The per-owner staging uses existing backend union capacity. The linked `original_bass` pool symbol remains 10,368 B, and `original_fx` remains 8,504 B.

| Stock target | Packaged app image | Free XIP | Beyond required 1,024 B XIP | Image growth | Free pool | Margin over 8,192 B |
|---|---:|---:|---:|---:|---:|---:|
| Elite | 551,536 B | 1,136 B | 112 B | 384 B | 8,972 B | 780 B |
| Pro | 551,524 B | 1,148 B | 124 B | 384 B | 8,972 B | 780 B |

Both ordinary RAM extents are 97,204 of 98,304 B and XPOOL is fully occupied at 9,216 B. After the required additional 512 B pool margin, the headroom is 268 B. These results are paired stock native links with the API retained, not target execution, timing qualification or full sound acceptance.

## Reproduce

Create a clean copy of the named compact source by applying the recorded stock-fit patch sequence 01, 02, 03b and 04 to `0cddaeb822458fdf4ed3b0b9035fcf22248fda10`. Apply `typed-edits.patch` to that composition. The host build requires the normal pinned fixture inputs and stock generated headers. Supply the read-only original `J6presets.zip` (SHA-256 `ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8`), the candidate's path and a fresh output directory.

```sh
python3 replay.py /path/to/typed-candidate /tmp/sj6-typed --banks-zip /path/to/J6presets.zip
cmp results.json /tmp/sj6-typed/results.json
cmp factory.log /tmp/sj6-typed/factory.log
cmp fingerprints.json /tmp/sj6-typed/fingerprints.json
```

For native sizing, apply `native-measurement-only.patch` to a private copy of the typed candidate. Build in sequence with the pinned toolchain/SDK and stock physical linker using `BOARD=smk37 MODEL=elite sh build.sh` and `BOARD=smk37 MODEL=pro sh build.sh`. The model receipts include linked section counts, the retained symbol and output file hashes.
