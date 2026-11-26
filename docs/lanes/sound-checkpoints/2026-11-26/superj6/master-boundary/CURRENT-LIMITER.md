# Current SuperJ6: dry-route graph checkpoint
Pinned installed Mac build 9. Inspect the ARM64 slice with `otool -arch arm64 -tV`
and `-ov`. Instrument fields follow the exact `KoFMInstrument` Objective-C
layout in this image. Do not transfer the original app into the repository.

The shared path includes an `AKDynamicsProcessor` branch that takes the delay
node at +0xa8 as its input and is connected into the reverb mixer at +0x70
(initialization around 0x10005a284–0x10005a328). The reverb mixer goes to
master amplifier +0xd0, then master attenuation +0xc8, amp tracker +0xb8,
then final DC blocker +0xd8 (0x10005a348–0x10005a44c). The initializer later
writes amplifier gain 0.85 (0x10005aa98–0x10005aaa8), not 4. At
0x10005ab00–0x10005ab50 it sets the dynamics processor's attack 1 ms,
release 50 ms, threshold −12 dB, headroom 0.1 dB, and master gain +1 dB.
Selectors were resolved through the actual ARM64 Objective-C stub relocation.
These are initialization settings, not a runtime signal or bypass readback.
Do not call the postcrusher suffix an exact whole graph.

The same Mac session enumerated **zero** components via AudioComponentFindNext,
including the known native Apple HighPass and Dynamics units as well as
SuperJ6. This isolates the direct-capture access failure to the session's
component registry, not merely a missing SuperJ6 plug-in. Earlier authenticated
same-build captures remain valid. An independent older-build test verifies
the final DC block has no hidden gain. None of these observations explains
the dry 12 dB level gap by itself.
