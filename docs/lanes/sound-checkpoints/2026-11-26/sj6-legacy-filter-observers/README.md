# Current SuperJ6 public legacy filter observers

The authenticated current ARM64 image (same source pin as
../sj6-osc2-source and sj6-current-faders probe) registers public
129/130 independently and stores both in the preset model. The reached
observer callbacks do not hit their historical sampler filter properties.

| Public ID | Current registration and store | Observer call | Actual behavior in scoped path |
| --- | --- | --- | --- |
| 7 | Global .82eb90 | .273528 → .2735ac → .0eeca8 | Actual cutoff receiver, distinct |
| 8 | Global .82eb98 | .2735dc → .273660 → .0efd74 | Actual resonance receiver, distinct |
| 129 | .26550c/.26552c, store .26553c, Global .82ee80 from .2654a8 | .273a60 → .0f9614 | Shared no-DSP callback |
| 130 | .2655a8/.2655c8, store .2655d8, Global .82ee88 from .265544 | .273a90/.273b10 → .0f9614 | Shared no-DSP callback |

The registration routine .01db44 and cold preset list .25ec10/.25ec20
confirm the two distinct public values. Actual execution of the shared
observer .0f9614 reaches .0f9518/.0f951c with a live three-child
original Sub/OSC1/OSC2 Swift sampler array. Values 0, .25, .75, and 1
all produce the same 65 guest instructions, exactly three iterator
advances at .0f95ac, zero sampler/core/conductor/instance writes, zero
new Objective-C receiver calls, and no unknown imports. This is actual
original guest code at the current observer, not a projection based on
metadata or a null receiver. Alternate legacy/public ingress outside
this witnessed path was not tested.

Earlier .addr6 and .addr8 sampler property receipts are actual internal
controls, but not evidence that either is reached by these current public
observers. Keep 129/130 separate and preserve their values. Do not alias
them to 7/8 or invent audible firmware behavior from registration alone.
No original assets, firmware changes, or production adoption.

Reproduce in the intact authorized bench with the same environment as
sj6-current-faders. The sibling probe below is the traced setup and
assertions; this checkout stores it but does not contain the original app.
