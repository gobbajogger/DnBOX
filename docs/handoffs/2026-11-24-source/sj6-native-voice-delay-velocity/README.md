# Typed controls combined native fit, no runtime tests

Baseline: exact `full-gate-inputs/sj6-native-voice-trio/replay-combined.sh`
source before its fixture copy, including the AutoPan native arithmetic,
Synth One PB/MOD prerequisites, SuperJ6 Mono/Legato/Glide and positive Gate.
`PREIMAGE.sha256` pins all changed pre-existing paths. Apply `delta.patch`
with fuzz=0 and verify `POSTIMAGE.sha256` and full `FIRMWARE.sha256`.
No runtime rehearsal, emulator, shipping files, device or Git changes.

Source merged: S1 Delay Feedback b30 + input cutoff tracking b39 (K1/K2
on one DELAY page), SuperJ6 Filter Velocity b2 exact finite FMA (existing
FILTER MOD UI plus queued owner edit, preset/base reload, note-lock/base
restore and Undo hook; unsupported nonzero typed note-lock still refuses),
and SuperJ6 RANDOM2 b26 exact native carrier. Sidecar owner fields,
project read-only policy, prior VOICE undo guards, legacy Gate b5 and
Random2 canonical nonzero opt-in guards survive the explicit merge.
No stock activation or claim of tested compatibility on this exact mix.

Exact guarded native source link (not product fit): Elite 552,348 B of
552,672, Pro 552,336 B; 324/336 B remaining. POOL 504 B after 8-KiB
reserve on both. The prior 55,307 B D9 lab companion conflicts with
installed S1PK, so this is not an installable profile. Remaining controls
need real capacity or opt-in architecture work and target adapters.
Specifically SuperJ6 ID89 needs original ID181 normal-cutoff adoption
currently inside the unsupported typed MOD selector, ID91 needs a real
native PWM route, and PITCH mapper's ready detent leaf alone is larger
than current headroom and does not implement its hot waveform or owner routing.
