# Original Synth One step-sequencer diagnostic

The authored arpeggiator and sequencer flags are enabled in 270 of the 694 checked programs. This focused replay selects five close failures and four existing stereo passes. The original observer logs the ordered note-on and note-off callbacks at the actual render frame while leaving the original render unchanged. The firmware fixture substitutes these events at the voice boundary and keeps the same 40-control projection, existing renderer and effects. A separate split-only control renders the direct inputs with the same callback partitions as the substitution.

The original PCM hashes, direct firmware PCM hashes, and all 40 control readbacks matched the full matrix for all nine programs. The split-only firmware PCM matched the direct firmware PCM byte for byte on all nine. Original note substitution qualified #482, Soi ARP 17, with 0.314/0.250 dB absolute level error and 1.661/1.659 dB spectral error. It failed all four pass guards, #10, #221, #213 and #481. The shared sequencer timing alone does not preserve the existing passes. No runtime route was changed.

Use the verified original factory build and the `panel-targets.f32` produced by the existing `replay.py --crush-lfo --voice --sub` invocation for [the 40-control matrix](../../../../evidence/sound-resume/synthone-pitch-lfo/). The runner pins the target binary data and uses the archived factory row hashes.

```sh
python experiments/sound-hardware-v1/synthone-factory-projection/sequence-diagnostic/run.py \
  REPO VERIFIED_ORIGINAL_FACTORY_BUILD PANEL_TARGETS NEW_OUTPUT
cmp NEW_OUTPUT/focused-results.json \
  experiments/sound-hardware-v1/synthone-factory-projection/sequence-diagnostic/focused-results.json
```

The [focused receipt](focused-results.json) includes the ordered events, per-channel scores and output digests. Raw audio stays in the local replay output.
