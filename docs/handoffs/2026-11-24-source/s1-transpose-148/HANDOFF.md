# Original Synth One keyboard TRANSPOSE (= ID148, not ID31)

Pinned AudioKit/AudioKitSynthOne 6466a3715c96b7ecf1dd255a218cbd571408a314:
Manager.swift sets the visible transposeStepper bounds from the parameter;
Manager+callbacks.swift binds its value to .transpose and the keyboard
display. HeaderViewContoller.swift exposes a Transpose readout. S1Parameter.h
assigns 148, S1DSPKernel.hpp sets signed integer −24…+24, default 0.
The toggleKeys/startStop notes code changes only tuning lookup (raw input
root retained); the lookup clamps to MIDI 0…127. Existing sounding pitch is
copied into the active voice, while mono fallback recalculates a remaining
held root using the current value. The compact existing receiver already
uses the same note/root law and the A4 patch remains intact. ID31 is not
repurposed.

This fenced host-only control uses existing typed Synth One TUNING K2
TRANSPOSE (signed st). K1 A REF, existing clock and MOD roles, old note lock
state, oscillator coarse and pattern transpose are untouched. Tuning affects
the next onset including mono fallback, not the pitch of held sounding notes.
It is not a new frozen note lock payload. Test shows independent sequence
pattern +12 and Synth One +12 stacking at different stages in a real note.
No unrelated effect/preset/clock storage change.

Only authorized typed byte b15 claims signed value, zero neutral and 1…48
remaining detents. Legacy and non-active owner controls absent; malformed
values refuse, and recovered previous tuple is readable and not editable.
Real physical UI Undo/Redo, packed/staged save/reopen, both MIDI directions,
mono fallback and pattern compose. Untouched older reader verified separately
using actual new-writer media. No product defaults, native fit/timing or
release. Replay from here: sh replay.sh ../controls-init-only/source-mod-wheels
