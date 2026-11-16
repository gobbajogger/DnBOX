# Original Synth One FX resource probe

Current pre-FX owner: 7,972 bytes on pi32v2. This is **not** the full FX owner.
A host-compiled original-source diagnostic reports:

| Rate | Four original delay arrays | Reverb array | Stereo widen array |
|---|---:|---:|---:|
| 44,100 | 7,056,016 B | 98,760 B | 8,820 B |
| 48,000 | 7,680,016 B | 107,440 B | 9,600 B |

These omit source owner objects, phaser, filters, compressors, sine table and
frontend. Host sizeof(SourceControls)=256 and sizeof(SourceFx)=2064 are host
figures, not target layout. Exact diagnostic source:
`tests/fixtures/synthone-factory/frontend/fx_resource_probe.cpp`.

The original initialization uses four 10-second vdelay arrays, with each
allocation `(int32_t)(maxdel * sample_rate) + 1` float words. This is actual
source-backed host allocation, not a firmware link or on-device measurement.

## Reachable time, not just allocation capacity

An initial investigation accidentally inspected descriptor index36; that claim
is withdrawn. **delayTime is index43**, domain approximately0.000362812..2.5seconds.
The executed source frontend with tempo sync off clamps a request4.999 to2.5.
The original FX then receives L/R5seconds and RR/fill2.5seconds. Original feedback
domain is0..0.899999976. In the694 original non-User presets,17 stored delayTime
values exceed1second; maximum is2.499999762. Stored values alone do not prove the
resolved tempo-sync values, so they are not used to reduce allocation bounds.

A naive exact-capacity rewrite to these reachable maxima would still require
approximately2,880,000 bytes of float sample storage per owner at48k, **before**
cubic interpolation guards and reverb. That is an estimate, not a proven minimal
representation or an admitted optimization. Ring-wrap/interpolation semantics
and dynamic time changes must be preserved and independently compared before
any capacity reduction. No implementation has shortened delays, reduced
polyphony, added PSRAM or changed protected flash layout to claim a pass.

Integ separately measured combined application free BSS1080B, pool reserve
margin836B, free XIP2220/2232B at09f251. Neither the literal source FX arrays nor
the estimated shorter rings fit that application. Full FX/native admission and
all sound-lane completion claims remain open. Preserve behavior and escalate a
necessary design change rather than quietly substitute a smaller effect.
