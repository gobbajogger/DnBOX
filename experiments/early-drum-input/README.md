# Early physical 909 input, diagnostic only

`X0X_D9_EARLY_HIT=1` enables a deliberately narrow route while blocking
on foreground LCD transfers. It is not a release alternative. It admits a
single ordinary physical 909 strike only after the whole scan and MIDI
service, never dispatches UI in TIMER5, and does not write the regular
single-producer CQ. Existing fallback owns all ambiguous gestures.

The scanner's receipt suppresses replay of the same held physical edge in the
foreground. That receipt is sampled with the key mask and remains valid
through a Panic until physical release. A pending mailbox blocks project
admission, and Panic cancels pending older work while the IRQs are excluded.

The retained traces independently record the first eight accepted scan
events and the first eight audio takes. Equal totals alone must not be
treated as equal admission or sample timing. Do not enable by default:
synchronous LCD eligibility can change with render occupancy, and this
experiment does not establish raw-versus-packed playback equivalence.

Run `tests/host/run_early_drum_guards.sh` and
`tests/host/run_early_drum_engine.sh` for the focused host checks. Native
replay must separately prove the route was actually used for matching
physical events and confirm no late blocks or lost/doubled hit.
