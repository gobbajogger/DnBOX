# SMK-37 canvas ownership and actual call/renderer list
No fitted global 82-row candidate yet. Native proof covers selected 80+58 paths while retaining 138 for others. All rows are physical 240 pixels unless noted, and all paths must retain existing foreground callbacks and fence any pending DMA source before writes.

| Entry | Renderer/file | Current 82-row readiness | Distinct state to freeze |
|---|---|---|---|
| Early boot | gfx.c boot image stock/custom | Separate verified 3×80; probe enabled | static accepted resource and material |
| Main Clip waiting | ui.c draw_main → paint_clip_wait | actual staged candidate; six palettes verified | frozen BG/white/gray and one-shot owner flag |
| Main Clip Grid | main_fm1.c smk_clip_render_strips → paint_clip_grid | actual staged candidate; exact callbacks, dirty and throttle checks under fixture | 132-B clip snapshot, bounded copy of IRQ-owned velocity and queued flags + view; mapping/hash after restore |
| Main Clip Keys | main_fm1.c ck_commit_page_strips → ck_paint_page_strips | actual staged candidate for Scale/Chord/ARP; host all 17 selected states | foreground field/page/keys, per-field values, derived colors; functions imitate old keyed black borders exactly |
| Clip knob row | main_fm1.c smk_ck_panel | already 82 | signature composed of settings, chord masks and arp step; keep completed DMA |
| Main Home patterns | ui.c draw_home / commit_home_strips | existing staged scoped default, native all variants not link | touched held readout still takes fallback; pattern glyph strengths and saved-state label |
| Live Home | ui.c draw_home_live | unconverted | scales/arp snapshots, current part-activity, live selection/readout; distinct from patterns |
| Song | ui.c draw_song / commit_song_strips | existing staged when no overlay or touched readout; cannot remove all fallback yet | visible bars/names, cue/playhead/touch readout; preserve C side reads |
| FX / Mix | ui.c draw_fx / draw_mix | unconverted | target values, wavetrace, master compressor hold, volume/FX history, palette; do stateful curve updates only once |
| Drum voice / Drum/Seq | ui.c draw_drum_voice / draw_drum (ui_drum_voice.inc) | unconverted | trigger/playhead, step settings, footer, envelopes/readouts, SMC lane, live meter; preserve compact native overlay state |
| Bass sound | ui.c draw_part_sound (engine-specific branches) | unconverted | all Chicago/Miami/Synth One/SuperJ6 control IDs, temporary arp and typed snapshot; avoid pointers across callback; approved live glyphs |
| Bass sequencer/generator | ui.c draw_303 | unconverted | selected step, note/slide/ARP positions, mode visuals |
| BREAK | ui.c draw_break / preview | unconverted | actual waveform/slot/meter, trim, live preview; no alias into sample/preset storage |
| PERF (flagged) | ui.c draw_perf | unconverted | timing snapshots; shared scratch belongs to active PAT test |
| Help, Ask, List | ui.c draw_help / draw_ask / draw_list | unconverted | Help state/text, selected/scroll action rows, knob overlay, any BLE peer live state; latch readout separately |
| SMC peer | smc_mixer.c smc_ui_draw_peer → smc_ui_grid | unconverted | must capture grid at exact point after header emit (callbacks may already have advanced state); eight resolved knobs/faders/pickup and button captions/values/ink; keep following knob and fill panel reads at their original point |
| Header/knobs/calibration/status | ui.c header and knobs; gfx.c text/fill | already fit 20 or ≤82 | pending full-canvas DMA fence preserved; modal restore/hash ownership separate |

## Bounds of sharing the driver
Actual compiler output for the V1 all-routes failing attempt includes 192 B commit_main_strips and 148 B commit_full_strips. Even deleting both entirely would recover only 340 B less shared replacement overhead, far short of the best 1,400/1,392 B link deficits. Boot uses different transfer windows, so it is not a third interchangeable 138-row commit. Not recoded as a stand-alone fix. Legacy Clip dispatch elimination already yielded 1,264 B by allowing old Clip renderer/control graph to be garbage-collected. Don't count that saving again.

A larger future reuse seam is renderers, not drivers: in the measured contained ELF the old Home and Song draw functions occupy at most 0x31a + 0x2c4 = 1,502 B, but these bytes are not free while mutable fallback/readout remain and replacement code is needed. Restructure only after exact frame models for those fallback scopes. The proposed full-screen wipe/recapture would change callback ordering mid-frame; not allowed.
