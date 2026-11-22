# Synth One voice and note-schedule controls

Production source: `2e6a69cc5229f8911b0158cae0f9a65356b41292`. Inputs and baseline: [the existing nine-program receipt](sequence-diagnostic/). The factory parity thresholds remain 1 dB level and 3 dB spectrum.

## Candidate voice control

The host-only variant uses voice zero on the authored mono programs. It retains oscillator and filter history across pitch changes, updates the pitch through the existing pitch function, and applies an eager release before retrigger when the authored mono mode requests it. A note-off releases voice zero. This probes gate/history behavior only. It does not implement the original portamento. The direct and captured-event controls without the variant matched their recorded PCM hashes. The tested arms retained the existing 40 control readbacks.

| Firmware input | Existing voice policy | Authored-mono test policy |
| --- | --- | --- |
| Direct notes | #10, #221, #213, #481 | #10, #221 |
| Captured original sequencer notes | #482 | none |

Programs #139, #664, #410 and #138 failed in every arm. Only the five authored mono programs receive the test policy.

## Original-renderer schedule control

The original observer preserved the preset controls, voice DSP, effects, held-note inputs, and sequencer clock. In the direct arm it intercepted the sequencer's voice callbacks and routed the direct fixture notes through those same original callbacks at frames 0, 22050, 44100 and 66150. The authored original control matched the nine prior PCM and event hashes.

| Original note schedule | Firmware direct notes | Firmware captured sequencer notes |
| --- | --- | --- |
| Authored sequencer | #10, #221, #213, #481 | #482 |
| Direct fixture notes | none | #482 |

The two mismatched-schedule pairings pass for #482 while both matched-schedule pairings fail. The prior #482 improvement cannot establish voice parity. Neither test policy enters production. The shipping 10/694 result and qualified stock packages remain applicable.

Both host experiments completed and returned the pass sets above. The execution environment disconnected during the follow-up packaging command. This receipt records the observed outcomes. The generated replay sources, full score matrices and local follow-up commit could not be verified after disconnection.
