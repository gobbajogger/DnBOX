# SuperJ6 ARP / SEQ original control index and native live fixes
Static original Build9 AU SHA256 a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1. Original runtime/metadata source only. Do not distribute private references. This work did not compile firmware, run fit, execute the app, or run host/playback tests.

Apply `live-edit.patch` then `held-resume.patch`. Both are source-only. Controls remain on the real current scheduler. The first closes the control-path reset: Type, Octave, Interval, Rate/Division, Length, step pitch and step gates update a running SuperJ6 while keeping the current note, phase, index, pending release and held order. Coalesced Gate/period changes latch an old Note Off even when aggregate fields are edited together. A true ARP↔SEQ/off transition still resets phrase ownership. The second keeps recent physical live keys in the already-existing 8-note scheduler cache while bypassed. Releasing ARP/SEQ restores captured per-note velocities in physical press order (oldest first to respect Mono priority), instead of giving all resumed notes the last velocity; no additional carrier. Keys beyond the existing bounded cache are sent a fallback attack on exit. Native transport-triggered bass notes always bypass the live key cache.

| Original Build9 user control | Native receiver and real action |
|---|---|
| 63 ARP Enable | ARPEGGIATOR K1 → ARP_F_ON; turning on shuts off 142 |
| 64 ARP Type, original slider setter 0x1000eeda0 | ARPEGGIATOR K3 → ARP_F_MODE; seven original headings, no alias; Random types 4/5/6 actual graph |
| 65 ARP Octave 0..3, 0x1000eeddc | ARPEGGIATOR K2 → ARP_F_RANGE, project stores physical octave count 1..4 |
| 66 ARP Interval 0..12, setter 0x1000eee18 | ARP SEQ K2 → ARP_F_INTERVAL |
| 142 SEQ Enable, 0x1000f40dc | SEQ SETUP K4 → ARP_F_SEQ_ENABLE; turning on shuts off 63; distinct selector retained |
| 143 Division and detented ARP Rate, 0x1000f4380, 0x10027a2f0 | ARP SEQ K4 (BEATS) and ARPEGGIATOR K4 (SPEED) → shared actual period; 13 original normalized positions (5/17 through 17/17) |
| 144 Seq Gate 0..100, 0x1000f4254 | ARP SEQ K3; shared duration for all steps. Literal zero emits real onset then scheduler release; not an invented per-step Gate length or rest. |
| 145 Seq Length 1..16, 0x1000f4464 | ARP SEQ K1 → ARP_F_LENGTH; no forced note-off on edit |
| 146 Seq Destination, 0x1000f42c0 and 0x1002efc44 | SEQ SETUP K3 → real downstream oscillator/amp+filter mask BOTH/OSC1/OSC2 (0xF/0xD/0xA in Build9). Current routed On uses this for each next generated event. |
| 147–162 16 Step Pitch offsets, first setter 0x1000f44c4 | Pads/held pad + keyboard → ARP_S_OFFSET; -24..24, not a piano roll |
| 163–178 16 Step toggles | Existing pad grid → ARP_S_ON; independent true silent steps, not duplicate duration knobs |
| Start Snap and Lock (distinct Conductor settings, not IDs 146 or 118) | SEQ SETUP K1/K2; start follows available external MIDI boundary; Lock preserves authored sequence subtuple on replacement |
| Global transport BPM and following MIDI clock | Existing project/MIX TEMPO actual `seq.bpm` and global MIDI + CLOCK source AUTO/INTERNAL/EXTERNAL; two synth owners use transport tempo, typed Start Snap sees live external grid. Original AUv3 host and Ableton Link are platform-specific inputs, not false aliases. |
| Tempo popup “load tempo with preset” / tempo lock | Original separate host preference; no equivalent required for this no-factory-bank build, and not aliases for Start Snap; resolve preset-import policy in that later scope. |
| 67 Sync Enable; 118 Tempo Sync Enable | Do not count as additional ARP clock knobs. 118 is the independently authenticated FX/Delay sync route. 67 is outside this source slice. |

Original destination in-flight behavior remains partially known. The setter applies the changed original four-lane mask XOR to 0x1002eda94 and then tail-calls 0x1002ee9ec. 0x1002eda94 traverses tracked note records, checks active masks, and calls layer note-off via 0x1002f1044→0x1003db5a0. The latter sets separate OSC and AMP-layer releases; it is not a global all-off or an ordinary retrigger. The current compact voice has one gate/note owner across both oscillator layers, not separate original note records and four gate bits. Deliberately did not add a fake restart or treat subsequent-note routing as exact in-flight equivalence. Any strict live-switch proof should match source direct-vs-sequenced note flags and shared-layer ownership before adding a DSP receiver. Likewise original app 128 physical held notes vs native bounded eight is still an explicitly limited corner, not claimed equivalent.
