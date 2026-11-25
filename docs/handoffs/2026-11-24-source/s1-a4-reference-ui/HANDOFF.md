# Synth One TUNING / A REF: same-layout host-only owner control

Original Synth One Wheel/Tune ID143 A4 is an integer 410–470 Hz reference.
This fenced actual SMK affordance reuses DnBOX TUNING, K1 A REF and "Hz".
It appears only for typed owner Synth One Init; legacy project and other
engines do not acquire a new binding. As with original, new 12ET onsets
capture the selected reference; a knob edit or history action does not
retune notes already held. No microtonal scale implementation or product
promise. No legacy object or factory preset changed.

Only private typed owner byte 6 is owned. Canonical zero is original 440 Hz;
codes 1..60 represent exact alternatives. Malformed code >60 is refused,
rank unambiguous; ID137/MOD/PB bytes and lock payloads remain unchanged.
A4 is owner tuning, not a new frozen property of expressive note locks.
No persistence merely from note or wheel movement. Same-version tuple
binding is inherited; actual recovered previous settings display honestly
and selector / undo / persistence on the fallback are blocked.

143 UBSAN host checks exercise real Select/K1 input on both owners, physical
HOME+REC/PLAY, owner-specific real MIDI and onset pitch including held notes
under undo, complete detent mapping, bounds/malformed validation, typed
writer and staged recall. Sidecar applies only through authenticated host
project ownership. The underlying receiver's all-MIDI/untouched PCM proof
is separately linked by exact dependency; not rerun as a native claim.

Replay: sh replay.sh ../controls-init-only/source-mod-wheels
No native/device/Git/publish/default-format changes. Source-only.
