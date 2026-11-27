# SuperJ6 Init-to-Init sound baseline

The compact renderer loads its own unmodified SuperJ6 Init. It does not load
Brainwave values, set master gain, or switch effects. The original side must
load the installed AU's actual User Bank “Your Init Preset” record, UUID
8733EFD7-4000-4C31-BC52-5AD9E4B707E1. That official record contains no explicit
parameter values; an empty authored list is not a vector of zeros. Capture
the real settled receiver values and render them unmodified.

One cue: 44,100 Hz, 148,684 frames, 1,164 callback blocks and 8 MIDI events,
including held single note, chord, and release. The compact host rerender
and event log are bit identical. It is centered at equal left/right,
0.098346 peak and −36.3453 dBFS RMS in the whole-note window.
This is not a statement of original parity.

Original attempt: the installed signed extension is version 67072 and the
matching Init record is present in its official User Bank. Swift compiled,
but the offline host could not instantiate `aumu/j6ct/AuPr`: OSStatus −3000
(`invalidComponentID`). Apple's own auval could not find the component
from the same session. No fresh original audio was emitted. No old preset,
approximate FX simulator or assumed zero defaults substituted. No app
registration, preset library or device settings altered.

## Reproduce

1. Use the original project cue handoff; run the existing
   `sj6-parity-suite/prep.py` for its exact C header.
2. `bash sj6-init-match/build.sh firmware-fit-next`
3. `sj6-init-match/render > compact.f32 2> compact.events`
4. In a Mac session that can already instantiate the installed original
   component, compile `CaptureInit.swift` with swiftc. Run
   `./capture "$OFFICIAL_USER_BANK" cue.json new-native-output` twice with
   different destination directories. The harness uses an offline transient
   AU instance (not a DAW or live device), exact Init identity, and actual
   control readbacks before and after the cue.
5. `python sj6-init-match/compare.py compact.f32 --log compact.events
   --native new-native-output --repeat new-repeat-output`.

The signed scorer rejects different cues, IDs, component builds, audio
hashes or native control readbacks. Report held/chord/release/whole with
no gain fitting or time shifts and include the original repeat floor.
Use effect-by-effect controlled cues on top of Init after the dry path
matches. An untouched Init match alone cannot qualify active effects.
