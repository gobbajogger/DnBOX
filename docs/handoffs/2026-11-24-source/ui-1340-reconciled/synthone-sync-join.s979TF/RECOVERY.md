# Pinned replay inputs

The current patch, fixture source and attached checksum files are the inputs for the joined Synth One host replay. The previous source-preservation checksum file at the checkpoint root covers the first preservation commit. For this replay packet use `sha256sum -c JOINED-SOURCE.sha256` instead.

- [Sound engine source and exact compile flags](https://github.com/gobbajogger/DnBOX/tree/c6c7862bd6c80af13cf459c9fa27c04fc0017390/docs/handoffs/2026-11-24-source/perf-final-proof)
- [Compacted PWM2 page prerequisites](https://github.com/gobbajogger/DnBOX/blob/d4c0c9a0ed9dcde635cf4613f8124c49f75974c2/docs/handoffs/2026-11-24-source/perf-final-proof/pwm1-lfo2-fit/page-matrix/HANDOFF.md)
- [Optional shared owner-pointer prerequisite](https://github.com/gobbajogger/DnBOX/blob/d4c0c9a0ed9dcde635cf4613f8124c49f75974c2/docs/handoffs/2026-11-24-source/perf-final-proof/pwm1-lfo2-fit/control-owner/HANDOFF.md)

For the exact host donor, enforce the preimage rather than trust an approximate revision. The generated host inputs and authenticated local Synth One pack are not part of this source-only handoff. This is not a target build or release receipt.
