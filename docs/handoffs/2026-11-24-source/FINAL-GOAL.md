# DnBOX final goal (work backward from this) — Orchestrator, 2026-11-23 16:50 VLAT

DONE means one thing: a combined Elite build and a combined Pro build from the
integration/dnbox-consolidated tips of both repos, in which every check below passes, ready for the
user to sign off to main. Each owner's job is to make their own rows true on that tip. Closing
tickets doesn't count.

Rules for getting there:
- Keep the tip green. If a red test on the tip blocks you, fix it in a small PR right away, even if
  you didn't break it. Don't wait, and don't route around it.
- Build the end state first, then fill in the details. Every PR should move at least one row from
  false to true, and should be merged within about an hour of starting.
- Report to Orchestrator only when a row turns true (give the SHA) or when you're blocked.

| # | End state on the consolidated tip (the check that proves it) | Owner |
|---|---|---|
| 1 | Elite + Pro build; host suite 0 FAIL / 0 not-run; flashmap pass; physically fits (XIP_MIN=0; build.py still fails a real overflow), pool margin >=512 B, no PSRAM | Perf |
| 2 | DROPPED 2026-11-23 23:36 (Dot bar: 10-minute heavy playback). audio-stress stays in the per-push gate | — |
| 3 | The user's 5 BREAK samples are in the stock firmware with preview-before-choose; /private-audio/ URLs return 200 | Perf (fw) + Web Remote (URLs) |
| 4 | Synth One: programs=694 nonfinite=0 xruns=0, every program selectable, FX on; old Drone projects load as Synth One Init | Synths |
| 5 | SuperJ6: 64/64 controls and every FX change the output; bypass bit-exact; presets=640 unmapped=0 (private copy, skips when absent) | Synths |
| 6 | A/B for Synth One + SuperJ6: every program within 1.0 dB RMS and 3.0 dB spectral | Synths |
| 7 | Every screen the walker reaches is R22 (the synth sequencer uses the R22 piano roll); label+glyph+value+unit; geometry 0 findings; text scan finds 0 explanatory strings or "?" | FW Core |
| 8 | Pan L50..C..R50; 4-level battery on every top bar; pads/LEDs drive 909/808/BREAK; GLOBE+pad clear/copy; slider-lane label switch (host tests) | FW Core |
| 9 | Glyphs: one shared 0.0.26-based glyph per parameter type, live and animated, pixel-matched to G-FINAL, ported in place; TAPE reels round; every envelope row in every engine uses draw_adsr_inline | FW Glyphs |
| 10 | DROPPED 2026-11-24 00:02 (Dot-only): Reference images committed only after checking them by eye | — |
| 11 | DROPPED 2026-11-24 00:02 (Dot-only): compact-programs clean under LeakSanitizer | — |
| 12 | Remote: FunkyBeats roll, step seq, mixer, clip launcher and synth editor in R22, gestures with no tool row, thin slider lane, fits 844x390 | Web Remote |
| 13 | Web: 19 routes build+test exit 0; installer, emulator, import/export browser tests pass; HTML UI within 1% of firmware at 240x240; musician copy; fits 844x390 | Web Remote |
| 14 | Drone grep 0 in both repos; no evidence-only files in Git; <=15 branches per repo (archive tags on all deleted ones); roadmap generated from CONSOLIDATED-STATE.md | Cleanup |
| 15 | gh-pages fast-forwarded; consolidated-to-main PRs open, never merged | Orchestrator, last |
| 16 | (user asks, from the USER-ASKS-AUDIT.md rows marked "not in FINAL-GOAL") SuperJ6 all 640 presets render on the ARM64 bench; every SuperJ6/Synth One parameter reachable on the 4 SMK parameter knobs (pages of 4, no extra steps; not one knob per parameter, same as other engines, user 22:11); SuperJ6 and Synth One arp/seq adjustable from the SMK pads (user 22:13); MIX master shows the compressor waveform as in DnBOX-MASTER-FILTER-labelled-40ms.gif; 909 Tune and Pitch don't duplicate each other; hi-hat stem drawn so it makes sense, locked by test; voice glyphs instead of BD/SD in sequencers; Chicago optimizations verified; MIDI export usable in Ableton | owners per USER-ASKS-AUDIT.md |

User decisions, 2026-11-23 23:27 VLAT (UTC+10): keep the Remote layout as it is (the header/tab-bar re-layout and the within-2%-of-OWNER-BASELINE-1836 check are dropped); TouchOSC is dropped for now — the user will do it later, reusing the Remote website, once working firmware supports it. Those two phrases were removed from row 12.

User, 2026-11-23 23:29 VLAT (UTC+10): "That baseline is just randomly appeared cause dot just imagined a lot of stuff instead of actually doing the job." OWNER-BASELINE-1836 was Dot's Remote draft, not a user target, and must not be used as a target anywhere.

## Provenance (audited 2026-11-23 23:32 VLAT)

Source of truth: `USER-MESSAGES-VERBATIM.md` (U001–U675, ~14–16 Nov) and memory facts that quote the user. "User-sourced" means the user asked for the substance. "Dot bar" means a numeric or procedural check Dot (or the brief formalized from Dot) invented on top of that ask. "Dot-only" means no user quote was found for the requirement itself. Dot-only items stay in the table except the two row-12 phrases removed above.

| # | Provenance |
|---|---|
| 1 | Mixed. "No PSRAM" / fit stock Elite and Pro: user, 2026-11-16 19:24. User 2026-11-24 00:42: the build just has to physically fit (XIP_MIN=0); pool margin >=512 B kept. Remaining formalizations: host suite 0 FAIL / 0 not-run; flashmap pass. |
| 2 | Mixed. "a performance of the firmware tested to have 0 dropouts" (U265, ~15 Nov). Dot bar: "10 simulated minutes of heavy playback". |
| 3 | Mixed. Five BREAK samples in the stock firmware, with preview-before-choose: U289, U290, ~15 Nov. Dot bar: "/private-audio/ URLs return 200". |
| 4 | Mixed. Replace Drone with Synth One, old projects load, a real port with FX, sound like the original app (U217, U224, U324, U327, U328; user 2026-11-23 19:53, 23:20). Dot bar: "programs=694 nonfinite=0 xruns=0" (694 is the factory bank size). |
| 5 | Mixed. Port the AudioKit / J6 presets and make them sound like that version; account for all SuperJ6 features (U001, U216, U328, U451). Dot bars: "64/64 controls"; "bypass bit-exact"; "presets=640 unmapped=0 (private copy, skips when absent)" (640 is the factory bank size). |
| 6 | Mixed. "a test bench for synth one" / "sound comparison for both" (U467–U468, ~15 Nov). Dot bar: "within 1.0 dB RMS and 3.0 dB spectral". |
| 7 | Mixed. Unified firmware UI, R22 everywhere, synth sequencer keeps the R22 piano roll, no unnecessary on-page instructions (U132, U665, U674; user 2026-11-17 15:45, 17:30 "Only for synth seq r22"). Dot/spec bars: "the walker reaches"; "geometry 0 findings". "label+glyph+value+unit" is the design-system formalization of the user's glyph + intuitive-UI asks. |
| 8 | User-sourced in substance: pan "R(n),C,L(n)" (U661, 16 Nov); battery on every top bar (U136, U231, U512; U232 wanted % inside — later RULES formalized a 4-level icon, with % once calibrated); pads/LEDs on 909/808/BREAK (U203, U329, U457–U458); hold-GLOBE + pad clear/copy for the current instrument (U459–U460); velocity/chance/pitch/pan lane (U574, then user 16 Nov 19:05: thin sliders, not faders). "4-level" and "slider-lane label switch" are formalizations, not raw user wording. |
| 9 | Mixed. Shared 0.0.25.1/0.0.26 glyphs, one per parameter type, live and value-driven (user 2026-11-17 18:01: "we should reuse glyphs for consistency"); TAPE reels as circles not squares (user 2026-11-17 17:32); every envelope row uses the Chicago `draw_adsr_inline` graph (U242, U333, U340–U343; user 2026-11-17 22:10). Dot/spec bar: "pixel-matched to G-FINAL, ported in place". |
| 10 | Dot-only process rule (DOT-BRIEF fwui 0). No user quote found for "reference images committed only after checking them by eye". |
| 11 | Dot-only. No user quote found for "compact-programs clean under LeakSanitizer". |
| 12 | Mixed, after the 23:27 cuts. FunkyBeats roll, step seq, mixer, clip launcher and synth editor in R22 (U514, U570–U576; user 2026-11-16 19:08). No tool row; edits/zoom/pan by gesture (user 2026-11-16 20:04, RULES r33). Thin per-step slider lane (user 2026-11-16 19:05, after U574 "fader like funky beats"). Landscape fit (U538, U675). Dot bar still in the row: "fits 844x390". Removed as Dot-only / disowned: "within 2% of OWNER-BASELINE-1836"; "Remote/TouchOSC 0 unbound". TouchOSC itself was a user ask (U003, ~14 Nov) and is paused by the user, 2026-11-23 23:27–23:28, to do later. |
| 13 | Mixed. Installer, emulator, import/export, musician-facing copy, landscape (U057, U058, U062, U209–U214, U510, U674, U675). Dot bars: "19 routes build+test exit 0"; "HTML UI within 1% of firmware at 240x240" (user asked for a 1:1 HTML firmware UI, U209/U212/U262, not a 1% pixel bar); "fits 844x390"; "#120 vote stored and counted" (Dot-only as a product requirement — U370 is about how to ask the user for an approval, not a public vote page). |
| 14 | Mixed. Remove Drone (U217, U324, U327). No evidence-only files in Git (user, 2026-11-17 planning: no evidence-only work). At most 15 branches per repo, with archive tags before delete (user, 2026-11-17 12:48). "Roadmap generated from CONSOLIDATED-STATE.md" is a process formalization of the user's 2026-11-16 19:18 consolidation ask. |
| 15 | User-sourced in substance: publish the nightly site; open, never merge, the consolidated-to-main PRs (hard limit: no push or merge to main, user 2026-11-16 19:29). "gh-pages fast-forwarded" is the method formalized in the brief. |
| 16 | User-sourced (quotes already in the row, plus): SuperJ6 full-preset bench (user 2026-11-23 10:10 and later: original app code, including LFOs/arp/seq/effects); SMK pages of 4 on the 4 parameter knobs (user 22:11); arp/seq on the SMK pads, including SuperJ6 16-step arp pattern (user 22:13, 22:39); MIX master compressor waveform (U526); 909 Tune and Pitch distinct (user 22:09; U465); hi-hat stem drawn so it makes sense (U531–U532); voice glyphs not BD/SD (U362, U374, U379); Chicago optimizations (U002); MIDI / Ableton export (U475). |

Dot-only requirements still in this roadmap (left in place except the row-12 cuts, the 2026-11-23 23:36 drop of row 2 and the 1 dB / 3 dB / G-FINAL pixel / HTML 1% bars, the 2026-11-24 00:02 drop of rows 10 and 11, and the 2026-11-24 00:42 drop of the XIP>=1024 floor): row 1 bars (host suite / flashmap / pool>=512); row 3 bar (/private-audio/ 200); row 4 bar (programs=694 nonfinite=0 xruns=0); row 5 bars (64/64, bypass bit-exact, unmapped=0); row 6 bar (1.0 dB RMS / 3.0 dB spectral); row 7 bar (geometry 0 findings); row 9 bar (pixel-matched to G-FINAL); row 12–13 bar (fits 844x390); row 13 bars (19 routes; HTML within 1% at 240x240).


- 2026-11-23 23:34 VLAT (Orchestrator): dropped "#120 vote stored and counted" from row 13; Dot misread U370 (approval votes, not a public vote page).

- 2026-11-23 23:36 VLAT (user, all cuts): (1) Synth One/SuperJ6 A/B = close enough by ear with compact reverb; the 1 dB / 3 dB per-program bar in row 6 is no longer required. (2) SuperJ6 bench: only LFO/arp/seq move to original code; reverb/delay approximations stay. (3) Row 9 glyphs: match reference GIFs by eye; no pixel match to G-FINAL. (4) Row 13 "HTML UI within 1%" and row 2 10-minute stress playback dropped. (5) Merged /help/ideas/ vote page stays; do not remove.

- 2026-11-24 00:02 user: rows 10 and 11 (Dot-only) dropped.

- 2026-11-24 00:42 user: (1) Drop the XIP_MIN 1024 B floor; the build just has to physically fit (XIP_MIN=0; build.py's real overflow check still fails the build). Keep POOL_MIN as is. (2) The test build will be published, when Orchestrator says so, by committing both .fwsc files to DnBOX-nightly gh-pages under artifacts/nightly-dev/<sha256>/, like on 17 Nov, without publish_nightly.py's receipt check. Do not publish until told.
- 2026-11-24 10:08 user: every published test/nightly build (Elite + Pro) must also appear in the nightly installer page's build list (selectable, marked test, stable default unchanged), not only on a standalone test-builds/ page. Applies to 0.0.27 and the final build.
