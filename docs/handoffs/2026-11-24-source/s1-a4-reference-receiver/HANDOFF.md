# Synth One A4 master-reference receiver, 12ET scope

The original app exposes the tuning reference. The pinned DSP sources
(S1Parameter.h and S1DSPKernel+getSetSynthParameters.mm) define ID143
and truncate it before range admission (410–470 Hz, Init 440). The
[developer's tuning panel documentation](https://github.com/AudioKit/AudioKitSynthOne/wiki/Tuning-Panel/f0b86b0fe7288e385fbeb4dbc052b6da18b9685d)
explains how that reference changes the table through middle C.
The pinned note-on sources copy the current table frequency into a
new voice; they do not retroactively retune held polyphonic notes.

Compact previously accepted ID143 into the frontend but discarded it
during note preparation. The isolated patch applies the supported 12ET
law in poly and mono note preparation and handles extreme finite incoming
values with clamp-before-integer conversion. It deliberately keeps the
compiled exact current 440 playback expression. It does not implement
custom tuning tables/microtonal scales, take over an on-unit control,
change project storage, or assert full application reproduction.

Fresh actual MIDI note queue and audio host fixture: 299 checks across
both owners, all 128 notes, independent references, held old pitch and
next-onset new pitch, octave points, clamped integer stops, no synthetic
record or writes. Untouched 4,096 stereo frames compare bit-for-bit.
Old source fails the real note-on assertion. The supported 12ET reference
test uses a 0.5 ppm numeric tolerance between compact Float32 and the
independent double mathematical oracle; no claim of original PCM identity.

Replay sh replay.sh /exact/source-s1-reverb-lfo. Exact head source tested,
after the original ID51 patch. No published firmware, vendor link or
physical device run.
