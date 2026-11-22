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

## Recycled voice history

The original note allocator changes frequency and gain without clearing the oscillator or filter objects. The firmware resets the selected voice when allocating a new pitch. The focused host replay tests main oscillator phase and filter history separately at that boundary. Voice selection, envelopes, controls and note events stay fixed. Replay the existing nine-program experiment first, then run:

```sh
python3 experiments/sound-hardware-v1/synthone-factory-projection/sequence-diagnostic/voice_history.py \
  VERIFIED_SEQUENCE_OUTPUT PANEL_TARGETS NEW_OUTPUT
```

The two variants preserve all nine direct-note PCM hashes. With the exact original events, both change PCM for #664, #10, #221 and #213 while matching the other five rows byte for byte. Neither changes any stereo pass result. The existing #482 result is unchanged. Retaining recycled state alone therefore does not qualify a candidate. [Per-arm PCM digests and scores](voice-history-result.json).

## Active low-pass transfer

The host link replaces only the original low-pass computation with the firmware's fitted coefficient law and recurrence. The original per-sample cutoff, resonance, excitation, note events, voice state and effects still drive the render. All nine programs select low-pass.

```sh
python3 experiments/sound-hardware-v1/synthone-factory-projection/sequence-diagnostic/ladder_probe.py \
  VERIFIED_SEQUENCE_OUTPUT FACTORY_BUILD NEW_OUTPUT
```

The original note-event hashes match in all nine rows. The transfer substitution changes PCM but stays within the existing parity bounds on all nine. The maximum spectral delta is below 0.004 dB, and the firmware comparison retains the same matched-event pass set. This rules out the low-pass approximation as the dominant residual in these nine programs. It does not rule out upstream cutoff or excitation differences. [Per-channel results](ladder-probe-result.json).
