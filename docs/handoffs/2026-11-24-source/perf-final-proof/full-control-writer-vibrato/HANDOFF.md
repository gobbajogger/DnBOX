# Combined original control source checkpoint: SuperJ6 vibrato

This is the next material full-control integration checkpoint, not the
previously sealed fit fixture or a release. Apply vibrato-joined-source.patch
to the exact ordinary CHORUS+FILTER+drum-descriptor source at the PREIMAGE
fingerprints. Hash the postimages and apply the original vibrato host-test
patch from the existing SuperJ6 Work packet separately.

The join preserves the existing S1 and J6 cold transaction owners and the
common table-based page emitter, moves only the two vibrato turns into the
admitted J6 owner transaction, scopes chorus turns to their original four
bits, and scopes the vibrato cell marker to Juno. Note-lock extents and
Synth One envelope/link/oscillator storage are unchanged. The VIBRATO page
remains next to CHORUS; PWM BASE still resolves to its selected physical
oscillator before the edit. Explicit zero is persisted in the existing row.

Fresh mixed source host tests passed: 1,766 actual vibrato checks and the
paired-engine storage, filter, chorus, PWM, tempo sync, Saga and held-pad arp
regressions. The cited chorus and Saga regression counters are the standard
public host runs, not their separate authorized factory-bank variants.
Untouched production source and previous sealed disposable image remain
separate. Native same-source app/POOL measurement is pending. Authored
original vibrato loading is a separate work patch, not claimed here. This
checkpoint does not claim all controls, native timing, installer, or release.
