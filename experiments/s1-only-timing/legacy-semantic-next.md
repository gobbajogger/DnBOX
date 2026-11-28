# Next legacy matrix path: snapshot inputs, not generic drawing

The existing Synth One snapshot already freezes viewport, 4 x 4 visible refs,
row titles, focus, palette, and 16 step gates/playhead. The probe extracts that
unchanged work as cv_capture_matrix_layout, then gathers Chicago/Miami state
without invoking the S1-only cell decoder or setting s1_draw_frame. Source:
live-display-legacy-semantic-probe.patch, applied after the existing 69-row
base and ordinary knob patch. Define DNBOX_LAB_LEGACY_SEMANTIC only in the
focused host fixture. No display dispatch/build flag is changed.

Separate logical needs and owners, as verified in the current source:

1. Capture layout, bank identity (original display/render paths use both
   engine_bass_bank and engine_bass_sound_bank), 16 cells + off-page effective
   raw controls and exact 32-B packed sound cell. Freeze live lock/motion
   preview mask + values once. Keep original_text_view and original_wave_view
   projection functions but give them the captured source and masked overrides.
   Chicago disables ARP subcells using the exact ARP-switch projection; the
   Chicago/Miami glyphs have secondary cutoff/resonance and oscillator waveform
   queries even if those refs aren't on the currently focused row. This is why
   the S1's 16 cell values alone are insufficient.
2. Snapshot engine_bass_voice_view (amp/mod, live state, held), ADSR gate and
   four native controls, engine_bass_env_view (cursor level/state),
   engine_bass_filter_view (Chicago depth), engine_bass_motion_view (phase,
   bipolar, current, target, active). Keep native render behavior untouched,
   redirect only matrix queries after capture through these exact copied
   outputs. Preserve engine validity bits. Stop on owner/cell generation
   mismatch; do not invent a fallback snapshot. Confirm owner switches in
   injected mutation tests. Juno needs its separate envelope_view if enabled;
   test plan stays on Chicago/Miami.
3. Snapshot preset name/status/index/pending/edited, and requested bank if
   shown on the route. Reuse existing matrix formatting and step footer rather
   than duplicating font operations. Refactor layout into an independently
   stored compact struct so the eventual legacy payload doesn't allocate
   irrelevant S1 patch/EG words. Hold the snapshot pointer only while painting;
   restore real input state before LCD commit without delaying DMA input.
4. Keep 69 rows as fallback until all source reads above and snapshot invalidity
   are handled, all legacy lanes pass pointer-free mutation tests, and both
   native SKUs link without regressions. Then test 48+21 MAIN and 48+34 footer
   without stack canvas spill. Legacy tape generic path is not selected:
   measured XIP linker misses are 2,768 B and 2,720 B after outlining.

Host fixture prototype: semantic/host/x0x_host_legacysemantic.c with
semantic/host/build_legacysemantic.sh under the parent scratch tree. It
compares all actual legacy control text/wave projection values vs the existing
live decoder after mutating route/theme and tearing away the original packed
cell. It does not yet route real legacy pixels through snapshot, and does not
establish a timing or linker improvement. No changes adopted.

Verified host outcome: 1,704 exact text/wave results over 84 populated layouts
after scene/cell mutation. Only actually painted active waveform rows are
compared (Chicago one, Miami three). Probe record: 1,348 host bytes including
the full reused S1 payload; decompose into a common compact layout before
shipping any legacy renderer.
