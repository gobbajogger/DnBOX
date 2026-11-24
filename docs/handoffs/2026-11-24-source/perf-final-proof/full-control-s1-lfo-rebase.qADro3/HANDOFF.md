# Synth One LFO rebase on the combined product

The [single source patch](s1-lfo-full-control.patch) is pinned to the root-owned `full-control-candidate` after the authored SuperJ6 vibrato and Saga toggle integration. The seed matched the combined packet's postimage. `PRODUCT-PREIMAGE.sha256` and `PRODUCT-POSTIMAGE.sha256` pin all 313 firmware files, and the shorter `PATCH-*.sha256` manifests pin the nine changed files. The work used private source copies.

The four original rate/shape controls and original global **SYNC TO ARP** are in the product's existing table-driven Synth One pages. The sync group follows the original per-synth arp BPM. OFF preserves the last quantized targets. The rebased setter keeps the root candidate's no-inline audio commit and its single shared-row owner check. The 15-byte Synth One record and the 8-byte exact SuperJ6 vibrato record use the existing rows 6/7 according to the engine and its validated marker. Legacy 11- and 13-byte Synth One records keep their wire representation. The authenticated SuperJ6 factory and Saga implementations are outside the patch.

## Host evidence

- **Both ownership orientations.** The 382-check mixed fixture validates both widths together, preserves the peer's entire shared row and both Saga grids, and exercises project round-trip, malformed marked data, unmarked suffix storage, and a suffix occupied after enqueue. Cross-engine undo and redo restore the exact tagged row and its corresponding owned ARP/AMP2 state. The mixed fixture saves an old-format control, a new base Sound record and a lock-only record with the authored peer still present.
- **Exact previous product reader.** It loads the mixed control and recalls the peer's authored vibrato and Saga mask. It rejects unsupported base and lock-only records for each orientation, returns the expected unsupported result on save, and keeps both the store write count and all six objects unchanged. The independent rate and sync projects also have old-reader checks.
- **Physical controls and audio.** The new Synth One UI fixture runs both owners through physical detents on free-running and synchronized factory presets. All four controls change their owner's held 4,096-frame PCM capture while the peer's pre-mix frames stay byte-identical. The rate oracle covers 256 free detents and 152 original division words.
- **Atomic clock rollback.** The authentic and controlled-wet fixtures compare all four targets, the mode and arp BPM before the actual owner render. They cover authored ON/OFF, reload, locked onset, the next unlocked note and transport stop. Removing only the two wet restores in an isolated mutant leaves auto-pan at 0.75 Hz instead of 0.73 Hz on the first released render. The positive candidate restores the whole group. The controlled test uses synthetic 0.73 Hz and 0.11 s words only inside its host instrumentation.
- **Neighbors.** INIT and all 694 untouched Synth One factory programs match the preimage byte-for-byte. Authored SuperJ6 replay checks all 640 vibrato rows on both owners and the 640-row Saga corpus. Existing FILTER, oscillator, mixed storage, SuperJ6 sync/PWM2, and ARP keyboard checks also run in the replay.

## Exact host replay

```bash
TMPDIR=/dev/shm bash replay-host.sh /path/to/full-control-candidate /path/to/s1.pack
```

For the two authenticated neighboring corpus fixtures, append the existing local vibrato fixture directory, the existing `sj6-factory` reader source directory, and the existing local Saga fixture directory. Their input hashes are in `original-host-inputs.sha256`. The replay copies the preimage and applies the source patch privately before compiling. Host instruments are added only to disposable test copies. Native layout, timing and installation are outside this evidence.
