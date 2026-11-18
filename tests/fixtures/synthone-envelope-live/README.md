# Immutable Synth One scalar observations

`reference_observations.f32le` contains seven mono binary32 scalar envelope
recordings at 44,100 Hz, 16,384 words each, with offsets/stimuli in
`observations.json` and the frozen recording plan in `case_plan.json`.
SHA-256: `2cafbc8eb4b7aba8e22e0820ce8116d9dc7a57e6f8c41ed162c44ed98c474cfe`.
The independently authored component uses these observations as test targets;
no original engine implementation is part of its source or build.

The three earlier static recordings remain in
`experiments/sound-hardware-v1/mapping/synthone_followup/implementer_brief`.
`tests/host/test_synthone_dnbx_envelope.py` checks both immutable binary hashes and all three metadata hashes
before case selection and comparison. Exact equality is required, with no time shift, gain fit,
or epsilon. The component boundary test is a separately declared API invariant,
not an additional original-app recording. These fixtures do not qualify an
entire instrument, coefficient preparation over all controls, target arithmetic,
voice retirement, FX, or live production activation.
