# Synth One original MIDI sustain-pedal listener

Exact firmware base: controls-init-only/source-all-wheel, one app engine file
changed. Authority: AudioKitSynthOne public source 6466a3715c96b7ecf1dd255a218cbd571408a314,
AudioKitSynthOne/MIDI/Manager+MIDIListener.swift lines 84–90. State table:
- When up, any nonzero CC64 presses; zero leaves it up.
- When down, the next CC64 releases, even a repeated nonzero packet.
This is the actual upstream else-if latch, not the usual level interpretation.
If retaining conventional level semantics, do not merge this exact patch.

The candidate retains the incoming raw trace, applies canonical 0/127 on the
S1 receive path, and adapts the one-instrument original latch independently
to DnBOX's two committed part owners. No changes to other engines, physical
wheel, storage, settings or UI. Existing restore and motion paths unchanged.

bash run.sh green exercises raw queue input, active keyboard NoteOn/Off on
each owner, accepted 1/2/63/64/126/127, repeat toggles for 1 and 127, zero
idempotence, peer owner, exact Sound and zero writes, and retained other
engine CC64 values. 96 checks pass; bash run.sh red reprobes expected old
failures. Virtual host ports and actual firmware dispatch/voice, not hardware.
