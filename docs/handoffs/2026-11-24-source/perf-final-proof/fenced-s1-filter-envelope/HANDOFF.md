# Fenced, control-only Synth One Filter Envelope LFO ID56

**Not original-equivalent, not a release candidate, no native fit/timing claim.** This exposes the existing compact envelope-cutoff receiver exactly as it already exists in shipping-style host configuration. It does not retime filters, change any DSP source, or resolve the separately reproduced original per-sample versus compact KR8 mismatch. See sibling `s1-filter-env-lfo-equivalence/HANDOFF.md`: authentic active preset on every 3584/3584 off-ticks had an 11.805 Hz maximum stale ID56-specific cutoff delta. The exact latest composed prototype ran that same diagnostic byte-for-byte identical (`PAT-COMPOSED-KR8.log`).

- Pinned standalone pre 02069b8e9ebdc013e8fdbe7d79e8508245aba57d1008b21182d8d20f81bd47b6; post d3beef2fd0031d462267fd690cc2de1826334228a2fb499bb1b18114a072fa7e.
- Patch SHA256 b252db7e252038f2caa988178009aef6e04c5a096267497d5d8baa1103f0a4c3.
- Exact later PAT input bound pre f7d17a572dc1affdde651cb8764857da92ddb6308a3194d2bb72618ecbf98c12 cleanly applies with no fuzz; post 81317cca867c9ba8de20d234d9d498d82bdcd7aa5e96198d6556d7a538170600. No edits to PAT implementation/root.

The 0..3 OFF/LFO1/LFO2/BOTH original IDs map to the live same four compact modes on FILTER MODE K4 (ENV LFO). The narrow K2 matrix-only abbreviation ADSR P-TRK prevents label collision in the four-column page; the actual full ADSR PITCH TRACK parameter identity, list visibility, value and ingress remain unchanged.

Storage extends the exact same S1-only (mark bit23, byte[30]) product namespace to `fm + 5*decay + 25*filterEnv`. Three independent optional five-state digits give 125 legal tuples. All prior codes 0..24 byte-identical; zero each digit does not mean OFF, it restores the actual authored factory word. Tagged codes 125..255, wrong ownership/marker combinations, and nonzero unmarked current cells reject. The original [25] route group, packed Legato/HP/etc, historical factory and note tail are unchanged. A fourth independent digit is impossible in the same byte (625 states >256); do not reserve another arbitrary flag for it.

## Exact primary replay and scope

`replay-host.sh EXACT_02069_SOURCE AUTHENTICATED_S1_PACK`: 572,215 positive checks, 356 previous-reader checks, expected red absent before edit. All four K4 detents on both physical owners, exact peer/phase/raw cutoff/raw ENV-mix values, authentic active ordinal81 changed 4096/4096 stereo output frames per mode and original zero-mix ordinal542 changed 0/4096. Positive includes save/load, actual held note locks and restoration, and exact first adjacent OFF→LFO1. All 15,625 joint tuples for the two route groups and all 512 marker/raw-byte combinations verified. Old Sound/lock shapes still accepted and tagged/untagged legacy S1 owners slots 0..3 retag identically; new forms fail before any write. All 695 untouched factory host PCM exact SHA256 2b7c2c740edb57be12652f58c81a538815dd21c1ca8f66b0971cad51df0f331f. Physical Select both synths/owners 15 pages both ways, 14 unaffected SuperJ6 native-renderer host LCDs exact; four new LCD states inspected and no neighboring label overlap.

`replay-retained.sh MAIN_PRIVATE_SCRATCH AUTHENTICATED_S1_PACK` supplements factory ordinal4 literal LFO2 → real adjacent BOTH → saved reload → factory/released-lock exact LFO2, plus retained FM, AMP Decay, and Legato. Only the retained test's old-invalid range changes to recognize the newly typed region; prior audio/state assertions and Legato fixture unchanged.

Do not use this source-only qualification to claim original sound parity, latest combined native timing/fit, ecosystem reader compatibility, or publication. No code/device/Git writes outside isolated test scratch and this evidence packet.
