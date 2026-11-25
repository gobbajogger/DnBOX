# Existing Synth One PITCH MOD: fourth encoder is MOD WHEEL

Host-only actual SMK affordance. Existing PITCH MOD grouped DETUNE, PITCH and
TREMOLO, with fourth slot unassigned. Keep existing bindings, pages, filter
LINK and all other engines. Typed active Synth One Init displays one label
MOD WHEEL and three detents CUTOFF, LFO1 RATE, LFO2 RATE matching Original
AudioKit Wheels selection. Existing four-column widget uses the normal
caption/value and rendering. Legacy shows the prior unassigned fourth slot.
A coherent typed generation admits the physical knob; a previous recovered
generation shows the saved value but no selector/undo/save write. No extra
alias, page, MIDI or Remote binding. Selection never replays wheel movement.

UBSAN physical SMK host run: 121 checks. Hardware Select encoder visits PITCH
MOD; K4 scoped on both owners, unchanged K1–K3 and exactly one FILTER LINK.
HOME+REC/PLAY and typed save/reopen restore both owners. While A's real ADC
wheel is held, selecting LFO2 produces zero parameter jump. A duplicate report
and visible part A→B do not edit; next actual ADC moves only A LFO2. Actual
MIDI, Init Cutoff endpoint, typed V3 freeze and real sequencer restore pass.
A test with two typed virtual writer generations plus mixed latest Sound and
stream recovers the prior display and denies actual K4 and ordinary save.

Replay from package directory: sh replay.sh ../controls-init-only/source-mod-wheels
Uses exact b71f366f plus shared ID137 parser, verified S1 receiver and sidecar,
and installer tuple binding. New delta touches only ui.c, a read-only accessor
in project.c and selector admission in original_presets.inc. No exact approved
visual screenshot, physical hardware, native fit, final preset or release
claim. Do not ship or enable by default.
