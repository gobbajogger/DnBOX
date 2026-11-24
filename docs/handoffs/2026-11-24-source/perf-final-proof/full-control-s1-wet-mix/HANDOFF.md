# Synth One Delay MIX and Reverb MIX

Apply `s1-wet-mix-full-control.patch` to the root-owned `full-control-candidate` source pinned by `PRODUCT-PREIMAGE.sha256`. The patch applies unchanged to the first A REF plus LFO donor and to the refreshed donor with the SuperJ6 two-word mailbox. The latter is the fully replayed integration input below. Both manifests cover 313 firmware files. Patch SHA-256 is `7a1e0b4b30c0a60414458985bd7b2524a8eede4f4a5fb84e0724597a2bd6ccb8`. Preimage manifest SHA-256 is `9c1bbddb46afbbebb33ee47339fa34fdd80cadfd06722fde2b94e9351eeaf6cd`. Postimage manifest SHA-256 is `518a2f2f09d1cebe5d4c49f7bb602c30af65edfffbdbf39ed86e462a0f2eda6d`.

The existing Synth One EFFECT card uses K1 for CRUSH RATE, K2 for DELAY MIX (original ID 44), and K3 for REVERB MIX (original ID 40). Both MIX controls use the existing 0–100% formatter and the compact DSP's live wet targets. The original Synth One parameter enum, 0–1 range and wet graph are in the project's preserved upstream closure under `s1-audit-data/original-inputs/closure/master/AudioKitSynthOne/DSP`. Delay and reverb enables, routing, reverb modulation and DSP memory partitions are inherited from the selected program. Each owner caches two exact factory MIX floats to restore after preset replacement or lock rollback.

Existing panel, filter and LFO records use their original bytes until a MIX edit claims a typed 15-byte overlay. The new marker uses an invalid zero legacy header. The remaining bits store all old optional digits and the two new optional MIX digits, with the top two bits reserved. A new byte is claimed only if it is free at both edit admission and commit. A packed record is admitted only with valid digits and at least one MIX override. Note locks use the same record in the existing 15-byte tail. Preset replacement and engine switches clear exactly the owned extent, preserving opaque suffix bytes and unrelated rows.

## Host verification

- 1,100 focused checks exercised physical edits, held PCM, bypass, owner isolation, stable delay pool/line sizes, factory precision on reselection, save/reload, stored note locks, unlocked onset, and transport-stop rollback. Every enabled edit changed 4,096 of 4,096 owner frames. Every bypassed edit changed zero frames. The peer's pre-mixer PCM stayed bit-identical across all eight runs.
- The exact previous product read the old LFO control and refused both new Sound and lock payloads. Each refusal preserved all six project objects byte-for-byte with zero writes. The focused and mixed tests ran 257 and 266 previous-reader checks.
- 418 checks exercised reversed Synth One/SuperJ6 owner assignment, exact authored SuperJ6 A REF and vibrato payloads, Saga ARP, marked and opaque storage, packed LFO edits and cross-engine undo/redo.
- The unchanged LFO and tempo-sync profiles passed 597 and 1,955 checks. The deliberate missing-pan/delay-restore negative tripped the expected rollback assertion. The existing filter profile passed 33,234 checks, including independent ADSR and LINK.
- INIT plus all 694 compact factory programs produced byte-identical PCM in a same-profile pre/post comparison of 1,024 frames per program. The common PCM digest is `2b7c2c740edb57be12652f58c81a538815dd21c1ca8f66b0971cad51df0f331f`. The production encoder matched the root oracle's golden record. The oracle passed UBSan across 83,205 paired states, 36 selector states and 1,642,800 LFO states.
- `s1-effect-init.png` and `s1-effect-100.png` are actual 240×240 host renders from INIT. They show 13% and 0% at factory state and 100% on both controls after physical turns.

## Replay

```sh
TMPDIR=/dev/shm bash replay-host.sh /path/to/full-control-candidate /path/to/authenticated/s1.pack
```

The replay verifies the source and pack digests, applies the one patch in a private copy, and runs the positive and negative hosts. `replay-host.log` records the verified run. Its two `FAIL` lines are the expected no-editor and clock-rollback mutation witnesses. To reproduce the LCD images, use `ui_s1_wet_lcd_test.c` in the same host profile with the unchanged filter fixture's helpers, as prepared by the replay for the wet fixture. The packet contains source fixtures, reports and INIT-only LCD images. Factory input and generated project/PCM data stay in private local storage.

This qualification is host-only. Native fit, XIP and release gates remain open.

