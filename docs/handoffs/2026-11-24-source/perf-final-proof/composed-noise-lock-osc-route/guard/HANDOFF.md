# Qualify note-lock extension ownership

One original_presets.inc hunk on exact canonical Noise 888724f2… .
Synth One Noise source positions LFO1/LFO2 use base-mask bit25
(legitimate position codes 2 and 3); the same physical bit denotes the
SuperJ6 extension only in a Juno-tagged cell. The generic lock capture
used to call the SuperJ6 tail copy on any record with that bit, even on
a Synth One base record, overwriting the correct packed HP/cutoff tail
and refusing capture. Bank-qualify both Juno modifications together.
No wire change, renderer, storage expansion, new flag, or algorithm.

Actual wrong-path host witness on exact prior source: both kinds enabled,
owner A Noise LFO1 + HP/cutoff, capture -1. Corrected 202 checks:
all eight physical noise selectors across two owners, exact contained
packed data, note validation, actual project save/reload. Prior exact
reader 236 checks opens all eight repaired old-wire projects with zero
load-time writes; old parser compatibility does not qualify old capture.

Read-only audit: note validity gates both Juno extra and authored bits by
parsed bank; physical restore gates the part kind; original adoption
and deferred target validation gate by Juno; preset-switch clearing
checks prior cell engine; shared extra dispatch uses engine qualification.
Juno-specific helpers operate only under these callers. The only
generic lock-tail copy missing that qualifier was capture.

Run bash replay-host.sh EXACT_NOISE_SOURCE (no special pack needed).
No native/device, external, or root tree writes.
