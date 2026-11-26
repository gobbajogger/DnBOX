# Native Synth One H16 Widen verification (emulator only)

Pinned linked diagnostic pair: `s1-organ-wet-recovery/native-widen-bfp/tree/widen-native/{elite,pro}/never-install.elf`. Despite parent folder name, the linked provider is the H16 50 ms right history, not the experimental BFP8 provider that overflowed on both boards. The stock-path baseline is `rehydrated-reverb/authored-wet/tree/labout/{elite,pro}/never-install.elf`. Both paths use independently guard-sealed overlays of the model-specific Build27 stock templates. No device writes, repository writes, or installable packages.

## Reproduction

1. In the project workspace, only the single authorized emulator owner runs `flock -x /tmp/dnbox-gate-firmware.lock python widen-native-verification/cold.py > widen-native-verification/cold.log`. This projects and executes the pinned Near owner cold image, without the guarded project command's old full replay switch. Its outputs supply the verified cold project plus unchanged tagged donor for the new snapshots.
2. Run `bash widen-native-verification/build-fixtures.sh`. Compiles source host stream fixture and target ABI offsets.
3. Run `bash widen-native-verification/run.sh > widen-native-verification/loaded.log`, and separately `bash widen-native-verification/run-baseline.sh > widen-native-verification/baseline.log`. Both use the same pinned original 922 events and offset 0, strict 360 MHz/no skip, 8,000 ms, and the same project stream and exact reserved owner tag. Seed fixture copies A Synth One and arp only from the checked donor. The active fixture sets room 96/127, delay off, and Widen on, plus its matching edit/checksum/tag.
4. Run `bash widen-native-verification/verify-results.sh elite pro` and `python widen-native-verification/match.py > widen-native-verification/matched.json`. This verifies identical seed sectors/scene per model, no virtual NOR modification/erases, live owner, term-stack canary, actual H16 Widen target/embedded ownership/ring progress. The ring sample stats are point witnesses at final RAM dump, not evidence of wet output equivalence.
5. Optional separately guarded H16 cold owner check follows the same recipe with `cold-trial.py`; this is transport-off by design.

A pass requires worst <=10,809 us, no late halves, XRUNs, or emulator skips. This is one diagnostic offset on an authored wet synthetic project, not a full release gate, canonical factory render, USB/Widen control audit, hardware capture, or the full delay parity result. The four actual post-boot DNR3 owner polls are treated as point witnesses; the older scanner retains its strict 5/5 boolean and emits false when the startup negotiation poll has no reply. Nothing in the workload or threshold was weakened to make a pass.

## Offset-zero matched loaded results

| Board | Old Widen max/late/XRUN | H16 Widen max/late/XRUN | Emulator skips |
|---|---:|---:|---:|
| Elite | 15,840 µs / 105 / 105 | 15,558 µs / 39 / 39 | 0 both |
| Pro | 15,801 µs / 108 / 108 | 15,793 µs / 53 / 53 | 0 both |

The prior Widen route also fails. The candidate saves 48 B XIP on each model with unchanged measured 564 B usable pool. It reduces observed XRUNs but does not satisfy timing. Do not merge or claim release fit based on this diagnostic. H16 live Widen=1.0; embedded ring positions 1,333 Elite and 780 Pro at 8 s and zero NOR changes. Histories are silent in those endpoint dumps; separate same-input host sound validation, not these snapshots, proves the audible effect behavior. All timing is from actual replay, not host computation.

The unpopulated H16 cold owner diagnostics independently passed boot on both boards, with no late/XRUN. They are transport-off and silent by definition; see `cold-trial.log`, not an audible cold/startup pass. Their 16 virtual byte changes are the expected cold-side owner marker modeled by the stock guard, which is separate from the loaded scene's zero changes.
