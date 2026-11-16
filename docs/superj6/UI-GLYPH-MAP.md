# SuperJ6 controls → shared glyph library

Source of controls: the 170 R008 control ids (`experiments/superj6-conductor-source-20261115/control_data.hpp`, prefix `neon` dropped).
Library: `/workspace/dnbox-mockups/history/SHARED-GLYPH-LIBRARY.md` (Lead's box; renderer table and G001–G082 audit). No new glyphs are proposed.

Mapped: 141. Exceptions (no shared fit): 29. TUNE rows are mapped to the existing DG_TUNE renderer but flagged for Lead because the library lists it for drums only.

| id | control | shared function (renderer) | glyph ids | note |
|---|---|---|---|---|
| 0 | volumeControl | LEVEL | G022 | pot meter with live envelope pulse |
| 1 | pulseVolume | LEVEL | G022 | pot meter with live envelope pulse |
| 3 | sawVolume | LEVEL | G022 | pot meter with live envelope pulse |
| 7 | cutoffLayer1 | FILTER | G033 | cutoff knee |
| 8 | resonanceLayer1 | FILTER | G032 | peak height |
| 11 | verbEnable | REV / LOW CUT | G055, G070 | official stamp / pot meter |
| 12 | verbTypeII | REV / LOW CUT | G055, G070 | official stamp / pot meter |
| 13 | verbSize | REV / LOW CUT | G055, G070 | official stamp / pot meter |
| 14 | verbLoCut | REV / LOW CUT | G055, G070 | official stamp / pot meter |
| 15 | verbMix | MIX | G066 | pot meter |
| 16 | stereoWiden | PAN | G072 | position readout |
| 17 | compressorAmount | COMP | G062 | pot meter; firmware knee cue orig_fx_glyph case 4 exists |
| 22 | autopanRate | AUTOPAN | G081 | reuse existing strip |
| 23 | autopanDepth | AUTOPAN | G081 | reuse existing strip |
| 24 | delayEnable | DLY | G056 | official stamp |
| 25 | delayTime | TIME (pot meter) | G052 | pot meter, not a delay plot |
| 26 | delayFeedback | FEEDBACK (pot meter) | G053 | pot meter |
| 27 | delayMix | MIX | G066 | pot meter |
| 28 | highPassCutoff | HPF / SUB | G034 | Juno HPF |
| 35 | glideTime | GLIDE | G036 (firmware slope renderer) | existing slope strip |
| 36 | attack | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 37 | decay | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 38 | sustain | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 39 | release | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 40 | chorusIEnable | CHORUS | G030 | STATIC lens pictogram (library flags it FAKE as a plot) |
| 41 | chorusIIEnable | CHORUS | G030 | STATIC lens pictogram (library flags it FAKE as a plot) |
| 42 | chorusRate | CHORUS | G030 | STATIC lens pictogram (library flags it FAKE as a plot) |
| 43 | chorusDepth | CHORUS | G030 | STATIC lens pictogram (library flags it FAKE as a plot) |
| 44 | chorusMix | MIX | G066 | pot meter |
| 57 | fmModCents | FM / XMOD | G045 | og_fm renderer |
| 60 | unisonDetune | TUNE (bipolar) | DG_TUNE renderer (no G-id for synths) | EXCEPTION candidate: library binds bipolar TUNE to 808/909; no synth TUNE glyph |
| 61 | unisonStereo | PAN | G072 | position readout |
| 63 | arpEnable | NOTE / GATE | G078, G079 | reuse existing strip |
| 64 | arpType | NOTE / GATE | G078, G079 | reuse existing strip |
| 65 | arpOctave | NOTE / GATE | G078, G079 | reuse existing strip |
| 66 | arpInterval | NOTE / GATE | G078, G079 | reuse existing strip |
| 72 | layer1Semi | TUNE (bipolar) | DG_TUNE renderer (no G-id for synths) | EXCEPTION candidate: library binds bipolar TUNE to 808/909; no synth TUNE glyph |
| 73 | layer2Semi | TUNE (bipolar) | DG_TUNE renderer (no G-id for synths) | EXCEPTION candidate: library binds bipolar TUNE to 808/909; no synth TUNE glyph |
| 74 | layer1Detune | TUNE (bipolar) | DG_TUNE renderer (no G-id for synths) | EXCEPTION candidate: library binds bipolar TUNE to 808/909; no synth TUNE glyph |
| 75 | layer2Detune | TUNE (bipolar) | DG_TUNE renderer (no G-id for synths) | EXCEPTION candidate: library binds bipolar TUNE to 808/909; no synth TUNE glyph |
| 78 | layer1PWM | WIDTH / PULSE | G026, G029 | width input of the WAVE renderer |
| 79 | layer2PWM | WIDTH / PULSE | G026, G029 | width input of the WAVE renderer |
| 82 | layer1Waveform | WAVE (Juno) | G023 | Juno-style WAVE renderer; SuperJ6 is listed as a WAVE (Juno) engine |
| 83 | layer2Waveform | WAVE (Juno) | G023 | Juno-style WAVE renderer; SuperJ6 is listed as a WAVE (Juno) engine |
| 86 | oscRange | TUNE (bipolar) | DG_TUNE renderer (no G-id for synths) | EXCEPTION candidate: library binds bipolar TUNE to 808/909; no synth TUNE glyph |
| 87 | lfo1Enable | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 88 | lfo1Rate | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 89 | lfo1Amount | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 91 | lfo1DelayTime | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 93 | lfo3DelayTime | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 94 | lfo1Waveform | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 95 | lfo2Enable | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 96 | lfo2Rate | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 97 | lfo2Amount | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 98 | lfo2Waveform | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 99 | lfo3Enable | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 100 | lfo3Rate | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 101 | lfo3Amount | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 102 | lfo3Waveform | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 103 | tremoloDepth | LFO | G027, G028 | tremolo/vibrato are LFO rate/depth inputs |
| 104 | tremoloRate | LFO | G027, G028 | tremolo/vibrato are LFO rate/depth inputs |
| 105 | vibratoSemitones | LFO | G027, G028 | tremolo/vibrato are LFO rate/depth inputs |
| 106 | vibratoRate | LFO | G027, G028 | tremolo/vibrato are LFO rate/depth inputs |
| 107 | crushEnable | BITE / GNAW / CRUSH | G044 | og_crush renderer |
| 108 | crushAmount | BITE / GNAW / CRUSH | G044 | og_crush renderer |
| 109 | crushBitRate | BITE / GNAW / CRUSH | G044 | og_crush renderer |
| 118 | isTempoSyncd | TEMPO | G073 | tempo-sync flag |
| 119 | attack2 | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 120 | decay2 | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 121 | sustain2 | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 122 | release2 | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 124 | globalBend | TUNE (bipolar) | DG_TUNE renderer (no G-id for synths) | EXCEPTION candidate: library binds bipolar TUNE to 808/909; no synth TUNE glyph |
| 125 | layer1Bend | TUNE (bipolar) | DG_TUNE renderer (no G-id for synths) | EXCEPTION candidate: library binds bipolar TUNE to 808/909; no synth TUNE glyph |
| 126 | layer2Bend | TUNE (bipolar) | DG_TUNE renderer (no G-id for synths) | EXCEPTION candidate: library binds bipolar TUNE to 808/909; no synth TUNE glyph |
| 129 | filterCutoff | FILTER | G033 | cutoff knee |
| 130 | filterRez | FILTER | G032 | peak height |
| 131 | filterTracking | FILTER (env knee) | G031 | env-amount input of FILTER |
| 132 | filterAttack | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 133 | filterDecay | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 134 | filterSustain | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 135 | filterRelease | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 136 | filterStrength | FILTER (env knee) | G031 | env-amount input of FILTER |
| 137 | filterVelocity | FILTER (env knee) | G031 | env-amount input of FILTER |
| 138 | pitchAttack | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 139 | pitchDecay | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 140 | pitchSustain | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 141 | pitchRelease | ADSR / ENV curve | G037 (Juno laws), G048, G049 | one ADSR renderer; SuperJ6 laws, not a new graph |
| 142 | seqEnable | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 143 | seqDivision | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 144 | seqGate | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 145 | seqTotalSteps | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 147 | seqStep1Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 148 | seqStep2Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 149 | seqStep3Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 150 | seqStep4Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 151 | seqStep5Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 152 | seqStep6Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 153 | seqStep7Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 154 | seqStep8Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 155 | seqStep9Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 156 | seqStep10Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 157 | seqStep11Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 158 | seqStep12Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 159 | seqStep13Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 160 | seqStep14Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 161 | seqStep15Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 162 | seqStep16Offset | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 163 | seqStep1Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 164 | seqStep2Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 165 | seqStep3Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 166 | seqStep4Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 167 | seqStep5Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 168 | seqStep6Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 169 | seqStep7Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 170 | seqStep8Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 171 | seqStep9Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 172 | seqStep10Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 173 | seqStep11Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 174 | seqStep12Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 175 | seqStep13Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 176 | seqStep14Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 177 | seqStep15Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 178 | seqStep16Toggle | NOTE / GATE / TEMPO | G073, G078, G079 | step-sequencer cells reuse existing strip |
| 192 | lfo1Mode | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 193 | lfo2Mode | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 194 | lfo3Mode | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 195 | lfo1HzEnable | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 196 | lfo2HzEnable | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 197 | lfo3HzEnable | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 198 | lfoSnapbackEnable | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 199 | lfo2SnapbackEnable | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 200 | lfo3SnapbackEnable | LFO | G027 (rate), G028 (depth) | one LFO renderer, rate and depth inputs |
| 201 | noiseVolume | LEVEL | G022 | pot meter with live envelope pulse |
| 202 | subVolume | HPF / SUB | G035 | Juno SUB renderer (sub level) |
| 206 | aTuning | TUNE (bipolar) | DG_TUNE renderer (no G-id for synths) | EXCEPTION candidate: library binds bipolar TUNE to 808/909; no synth TUNE glyph |
| 208 | chorusVersion | CHORUS | G030 | STATIC lens pictogram (library flags it FAKE as a plot) |
| 210 | layer1PWMEnable | WIDTH / PULSE | G026, G029 | width input of the WAVE renderer |
| 211 | layer2PWMEnable | WIDTH / PULSE | G026, G029 | width input of the WAVE renderer |
| 215 | j60v2Rate | CHORUS | G030 | STATIC lens pictogram (library flags it FAKE as a plot) |
| 216 | j60v2Depth | CHORUS | G030 | STATIC lens pictogram (library flags it FAKE as a plot) |
| 217 | j60v2Mix | MIX | G066 | pot meter |

## Exceptions (no shared function; do not invent — Lead decides)

| id | control | why |
|---|---|---|
| 5 | layer1Sound | layer source selector (sample/osc pick); text readout |
| 6 | layer2Sound | layer source selector; text readout |
| 32 | monoEnable | switch / routing / mode value; text readout, no glyph function in the library |
| 33 | legatoEnable | switch / routing / mode value; text readout, no glyph function in the library |
| 59 | unison | switch / routing / mode value; text readout, no glyph function in the library |
| 62 | unisonType | switch / routing / mode value; text readout, no glyph function in the library |
| 115 | adsrLinkEnable | switch / routing / mode value; text readout, no glyph function in the library |
| 116 | adsrFilterLinkEnable | switch / routing / mode value; text readout, no glyph function in the library |
| 123 | vintageMode | switch / routing / mode value; text readout, no glyph function in the library |
| 179 | modwheelDestination | switch / routing / mode value; text readout, no glyph function in the library |
| 180 | lfo1Dest1BaseValue | switch / routing / mode value; text readout, no glyph function in the library |
| 181 | lfo1Dest1Param | switch / routing / mode value; text readout, no glyph function in the library |
| 182 | lfo1Dest2BaseValue | switch / routing / mode value; text readout, no glyph function in the library |
| 183 | lfo1Dest2Param | switch / routing / mode value; text readout, no glyph function in the library |
| 184 | lfo2Dest1BaseValue | switch / routing / mode value; text readout, no glyph function in the library |
| 185 | lfo2Dest1Param | switch / routing / mode value; text readout, no glyph function in the library |
| 186 | lfo2Dest2BaseValue | switch / routing / mode value; text readout, no glyph function in the library |
| 187 | lfo2Dest2Param | switch / routing / mode value; text readout, no glyph function in the library |
| 188 | lfo3Dest1BaseValue | switch / routing / mode value; text readout, no glyph function in the library |
| 189 | lfo3Dest1Param | switch / routing / mode value; text readout, no glyph function in the library |
| 190 | lfo3Dest2BaseValue | switch / routing / mode value; text readout, no glyph function in the library |
| 191 | lfo3Dest2Param | switch / routing / mode value; text readout, no glyph function in the library |
| 203 | pad1Destination | switch / routing / mode value; text readout, no glyph function in the library |
| 204 | pad2Destination | switch / routing / mode value; text readout, no glyph function in the library |
| 205 | fixedVelocityEnable | switch / routing / mode value; text readout, no glyph function in the library |
| 209 | filterType | switch / routing / mode value; text readout, no glyph function in the library |
| 212 | keyLockEnable | switch / routing / mode value; text readout, no glyph function in the library |
| 213 | keyLockRoot | switch / routing / mode value; text readout, no glyph function in the library |
| 214 | keyLockScale | switch / routing / mode value; text readout, no glyph function in the library |
