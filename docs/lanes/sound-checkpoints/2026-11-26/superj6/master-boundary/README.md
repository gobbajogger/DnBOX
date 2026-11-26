# Current Mac Brainwave level discrepancy across independent operators

From workspace root: `python sj6-newer-level-boundary/check.py`. The script reads
private known-input native leaf tapes and two independently captured full output
arms from the pinned installed Mac AU. It verifies their original PCM hashes,
scheduled notes, frame allocations, stable controls, and leaf input/output
custody. It replays the existing fixed source suffix without gain/phase fitting.
The archive and stored suffix hashes are included; the bundle stores no PCM.

| Authored window | Main reconstruction vs native | Sub reconstruction vs native |
|---|---:|---:|
| Attack | -12.519 dB | -11.945 dB |
| Held | -12.102 dB | -12.328 dB |
| Chord | -12.297 dB | -12.517 dB |
| Final release | -12.200 dB | -12.488 dB |

The Sub tape is an attribution arm with host HP upstream, not exact native HP.
Both reconstructed chains end at the source-backed guarded host suffix, not
the original app's internal mixer. Main-only saw phase or halfband changes
cannot fix both deficits. The independently driven Apple HP/Crusher outputs
have different measured level changes, and the residual is not proven to be a
single gain: the Main chord correlation is .25. No proposed gain or matching
claim.

## Minimal next discriminator (not executed here)

On the authorized current native AU, capture the exact same warm S4 cue in
two repeated isolated Main/Sub arms. For each, compare authored global master
to a single deliberately imposed 0.5× master-control value, retaining the
same authored lane faders, active modulation, mute matrix, and effects toggles.
Then repeat this exact pair on the native known-input prefix and same source
suffix, sharing controls, rates and callback tape. Compare raw terminal
waveforms and same-input ratios separately; do not infer an integer gain from
the existing deficit. A relative response matching both rates of master
control isolates that control law, not absolute gain. Definitive absolute-gain
localization requires the same immutable input at a real internal shared
oscSection/Crusher or pre-master tap, or a source-proof of its gain/normalizing
law. An external replacement node alone cannot establish an internal tap.
