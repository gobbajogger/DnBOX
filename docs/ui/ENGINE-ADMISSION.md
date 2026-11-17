# Current source discrepancies and admission requirements

The owner-authored `UI-SOURCE-OF-TRUTH.md` is preserved verbatim. Its required
future state is not a claim that the current firmware already implements it.
Reference version: owner document dated 2026-11-16 17:22; SHA256
`868957f911acd3d2de67f61db41f2f49a552d0f023d69be67df96a637149add6`.

## Current firmware, not renamed fiction

At development source `94197fc839b00aff47b98ca02f3c6396e4a6eabf`, and the local
R22 overlay under review, the selectable banks remain Chicago, Miami, Synth One
and generic Juno6. `firmware/src/app/original_sound.h` assigns the third slot as bank 2
and persistent identifier `0x4f533131` (OS11); historical `0x4f445231` loads as Synth One init, and generic Juno bank 3 and persistent
identifier `0x4f4a5531`. `original_sound.c` uses the kind in packed cell byte 29.
The UI selector and header names are in `ui.c`; runtime dispatch and compiled
union members are in `original_runtime.c` and `original_runtime.h`.

Synth One and real SuperJ6 replacement are required, but neither replacement
has been admitted to that combined product. Renaming either old bank would
misrepresent the audio engine and silently reinterpret saved data. Retain
legacy identity recognition and project bytes until an explicit tested
migration or safe unsupported-format rejection exists. Old captures and
fixtures are historical RED evidence, never current engine admission.

Generic Juno6's current pages are oscillator (Wave/Sub/Width/Level), filter
(Cutoff/Res/Env/HPF), envelope (A/D/S/R) and LFO/chorus. They are not the real
SuperJ6 parameter set and cannot be qualified by changing the displayed name.
The real SuperJ6 source has separate layer waveforms, PWM enables/widths,
sampler sub level and three separately typed LFO rate controls. A source-linked
exact-ID mapping and complete runtime/preset ownership remain prerequisites.

## Truthful glyph and storage constraints

- 909 kick Tune and Pitch are distinct controls. Tune is the pitch-sweep time
  constant in milliseconds; Pitch is a multiplier coupled to pitch depth.
  They require distinct names/units/glyphs, not a destructive parameter merge.
- Drum Attack is an amount. A normalized rising-edge symbol is not proof that
  the engine changes its actual rise time.
- An empty saved pattern must remain empty. Improve its meaningful grid,
  context and controls; never fabricate notes, playhead activity or meters.
- Missing native provider, unsupported engine routing, target timing, device
  behavior and incomplete page coverage remain NOT_RUN/FAIL as applicable.
- Existing approved tracked data stays unchanged. New private coefficients,
  fixtures and derived payloads are excluded from public source and artifacts.

## Gate coverage

The initial gate producer covers a fixed 45-state drum voice/page inventory
only. Other firmware engines, global screens, common-component tokens and
Web/Remote have explicit outstanding coverage. Source-token presence alone
must not turn a required per-page visual or engine-semantics check into PASS.
Every current page must eventually bind its source, controls, component tokens,
real render evidence, applicable interactions, old-defect fixtures and candid
human review to this canonical document. Full acceptance remains blocked.
