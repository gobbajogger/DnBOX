# Actual SuperJ6 Sub source: measured multirate, predictor, and restart cost

This experiment reads two exact resources present in the original app,
matching the prior private reference pins byte for byte. It does not
launch or modify the app and does not output original audio, decoded
patches, compressed media, or derivative tables. The source has 12
mono keyzones, source root labels 30…96 (not public MIDI), 44.1 kHz,
7,812,906 B PCM16. The authentic installed plugin also matches the
earlier pinned generated reference render. Synthetic codec variants
do not become original references. None was linked, shipped, or re-run
through the real instrument. Test input is deliberately not bundled.

Repro with NumPy/SciPy from this directory, substituting only paths to
authorized already-pinned resources:
```sh
python3 source_probe.py "$SFZ" "$ASSET" > source.json
python3 predictor_probe.py "$SFZ" "$ASSET" > predictor.json
python3 periodic_probe.py "$SFZ" "$ASSET" predictor.json > periodic.json
python3 restart_probe.py "$SFZ" "$ASSET" source.json > restart.json
```

Each source choice is measured with exact clock, no delay or gain fit:
strike, early body, disjoint loop head/tail, both sides of actual
declared loop and the whole waveform. The numeric dBr thresholds are
engineering screens, not release criteria. Plain best per-zone
packed/continuous Deflate at a **−40 dBr worst-window** screen needs
129,577 B even before frame restart, checksums, decoder, runtime,
presets or the required BREAK samples. At −30 dBr the best contiguous
stream sizes sum to 105,804 B, only conditionally below the legacy 112
KiB shared-pool number before accounting for actual live occupancies.
That stream has no random-access decode guarantee. On those exact
quantized words, independently restartable 32,768-word blocks already
need 182,833 B at −30 dBr and 235,586 B at −40, after a minimal index.
4,096-word blocks increase those to 585,453 B and 774,510 B. No claimed
spare provider extent or RAM budget was used to declare a pass.

Integer first/second differences, plane layout, and integer/fractional
pitch-period predictors round-trip every chosen quantized word but
did not beat the −40 whole-stream result. At −50/−60 dBr, a denser
quantization grid remains hundreds of KiB even before indexing; a
periodic predictor is not a sample replacement or a new sound claim.
All hashes/metrics in the archive are numeric or software evidence,
not source media. Use the archived Raw chord/release and owner
holdouts only if a genuinely smaller authorized candidate first
clears source and capacity screens.
