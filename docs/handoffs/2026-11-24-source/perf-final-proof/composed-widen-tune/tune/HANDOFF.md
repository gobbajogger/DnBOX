# SuperJ6 Build 9 layer 1 semitone, ID 72

Host-qualified source delta for one real control, not complete tuning, not authored preset integration, not native qualification. Exact parent: `perf-final-proof/composed-shape-voice.WVtz9o/source` (wet MIX + four-wave shape; **before** VOICE), all 313 product files checked against its `POST-SHAPE.sha256` with ledger digest `950af4ab6b7ad7f153e99a7a58157ddda87dbafd85d0189ed2896ed05cf7a0d0`. Source/tests are separate. The package contains no original AU bytes, presets, or generated release artifacts.

## Original contract

SuperJ6 0.999.8 Build 9, aumu/AuPr/jay6. Authorised AU SHA-256 `a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1`. Names/order/default come from `experiments/superj6-conductor-source-20261115/control_contract.json` pinned to DnBOX commit `5d18b182955d107c7f8ca09fb5b0b08a83a6adc9`. Setter and render routing were inspected statically on the pinned ARM64 image, not inferred by ear or from the mostly neutral archive.

| ID | Original owner | Exact finite normalized transfer, u∈[0,1] | Dispatch → setter | Scope in this delta |
|---|---|---|---|---|
| 72 | neonlayer1Semi | round-nearest ties away (24u − 12) semitones | 0x10026f584 → 0x1000f32b8 | physical edit / pitch / project / note lock |
| 73 | neonlayer2Semi | same law, separate layer 2 footprint | 0x10026f948 → 0x1000f32cc | not implemented |
| 74 | neonlayer1Detune | u − 0.5 semitones (±50 cents), no quantization | 0x10026f4d0 → 0x1000f3218 | not implemented |
| 75 | neonlayer2Detune | same fine law, separate layer 2 footprint | 0x10026f894 → 0x1000f3234 | not implemented |

Coarse common handler `0x1000f32e0` promotes the binary32 input to binary64, passes min −12 / max +12 / taper 1 to the scale helper, and rounds with FRINTA at `0x1000f3330` before updating the separate owner field and calling the tuning target. Fine common handler `0x1000f3500` receives −0.5/+0.5 from the two wrappers and writes the unquantized result; the four fields are distinct. The first-layer recomputation at `0x1002afbf0` adds independent pitch terms; second-layer enters separate wrapper `0x1002b36c0`. The shared linear scale helper is `0x1004902a8`. All four defaults are binary32 `0x3f000000`. The prior authenticated bank audit found 18 and 19 unique coarse words, respectively, with fine fields neutral in every record. This source does not implement decoding those authored ID 72–75 words. The new 25 physical knob stops are simply canonical discrete output choices, not a raw bank-float API.

## Scope and record

SuperJ6 TUNING K1, both physical owners independently; readout −12…+12 st. Mailbox 33 retains existing ownership fence, post-to-boundary semantics and project save/lock barriers. ID72 changes only oscillator layer 1's nominal semitone offset, composed with existing range, bend, and A REF. Intentional replay uses the compact DSP; no assertion of absolute Build 9 PCM parity. The marked extension stores only five spare upper bits in EXTRA bytes 2–6. Code 0 is canonical neutral/absent, positions 0…11 encode as codes 1…12, and positions 13…24 encode literally. Codes 25…31 are invalid. Existing low-seven-bit ADSR, remaining EXTRA and base bytes, including authored LFO, PWM and chorus, are not repurposed. EXTRA flag remains the owner; the editor will not claim a nonzero unowned row. No base-byte shape slot is taken. Legacy prior readers fail validation on tuned edited payloads rather than silently misreading them. Neutral reset clears the overlay and drops the EXTRA tag only if no other old edits remain. Tuning-only note locks intentionally do not trigger unrelated envelope reset.

## Recheck

Apply the source and test patches with `patch -p1` on a copy verified using `PREIMAGE.sha256`, and verify both postimage ledgers. Run `sh <packet>/host-replay.sh` from the resulting root. It expects the standard host break-bank include already generated and defaults to shift sanitizer. Main receipt: 1,202 assertions. Both held targets changed 8,192/8,192 frames, the other part changed zero; solo-layer rendering separately proves byte-exact PCM of untouched oscillator 2. All 25 positions, physical display, exact reload, real note lock onset and release, saved lock validation, neighbor bytes, and refusal of an unowned occupied row pass.

For downgrade qualification, build `ui_superj6_layer1_prior_reader_test.c` unchanged on both donor and candidate with `SMC_UI_TEST_FILE=<absolute path> HOST_EXTRA="-I<checkout>/tests/host"`. Run with environment unset on donor and `SJ6_NEW_READER=1` on target. For no-write comparison, run `SMC_UI_TEST_FILE=tests/host/ui_superj6_legacy_pcm_test.c HOST_EXTRA=-Wno-unused-variable` on both with separate output directories, and compare each `legacy.pcm`. Both exactly match SHA-256 `a3b1e885f4f553fb394991ba8c859c447f3d07770416db752db0a10340ded8bb` for 49,152 frames across SuperJ6, Synth One, Chicago and Miami. Existing A REF test passed 7,594 assertions and existing four-shape test passed every receiver/owner path. Replays use private bank input only when explicitly called for by the retained A REF fixture.

No git changes, external writes, hardware, native builds or target emulation done. Require combined product regression, exact native both-model footprint/cold/timing, and remaining field/storage mapping before promotion. Usable source packet only.
