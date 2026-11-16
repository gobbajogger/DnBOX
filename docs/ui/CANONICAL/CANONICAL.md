# Canonical firmware screens (the one reference set)

This directory is the **only** canonical reference for what each firmware screen and state looks like.
Every other capture or mockup is NON-CANONICAL (see `NON-CANONICAL.md`). Nothing was deleted.

## Sources and approval

- **target.png** — the newest approved design: Dot's **R22 gallery** (`/workspace/dnbox-mockups/from-dot/R22-gallery/views/*.svg`, rendered 480 px in `r23/_r22png/`, downscaled to the 240×240 panel). Approved: the user picked R22 for all glyphs on 2026-11-16. The user's approved deltas apply on top of every target: pan L50..C..R50 clip, per-voice pan, no explainer text, all glyphs live and animated, battery % in the top indicator. A target that already shows a delta's opposite (e.g. an R22 explainer footer) is overridden by the delta.
- **golden.png** — the deterministic host render of the firmware at the branch base `94197fc8` (`tests/host/ui_visual_gate/gate.py`, fixed clock, load 0). The visual gate diffs every build against it with tolerance 0.
- **status** — `MATCHES-TARGET` or `KNOWN-DEVIATION` (what differs, and the owning fix branch). Report: `gate.py --conformance`.

Target mapping: hand-mapped where an R22 presentation view exists for the state (909/808 voice pages, MIX parts/909/808/MASTER/WAVE, HOME, FX); otherwise the nearest R22 gallery view (recorded with its distance in `index.json`, to review).

## Coverage

777 states, enumerated from the firmware by `tests/host/ui_visual_gate/catalog.py` through real key/encoder input: every page of SEQ / sound (EDIT) / FX / MIX / HOME / TB-3PO for each of the 5 parts; every drum voice on SEQ and sound pages; a knob-touch readout and a held step per page family; every row of every SEL list and GLOBAL section; every button's help card; SONG; all 6 themes on each top-level view; transport play/REC; and every `shot` of the interaction scenarios (tour, song, help, sound matrices, page split, save feedback, groove, patterns, clock source, PERF, guard, jam, showcase, stereo), which reach the dialogs (CLEAR / RESET / LOAD / COPY / RESET SOUND), cue/chain, recording, motion, toasts and PERF results. A newly reachable page, list row or help card that is not here fails the gate (coverage check).

Not rendered by the FM-1 host sim (counted as **FAIL / not covered**): the SMK-37-only clip grid, WAITING FOR LOOPY PRO and pad-calibration screens (`BOARD_SMK37` in `draw_main`), and the SMC controller screen (`smc_mixer.c`, covered by its own framebuffer tests, not this gate).

## Conformance at base (golden vs target)

MATCHES-TARGET: 0; KNOWN-DEVIATION: 777.

| deviation | states | owner |
|---|---:|---|
| battery | 777 | R024 battery-% lane (branch not created yet; measured % or bars only) |
| footer | 714 | R22 no-explainer delta — lead/glyph-aa-ease-20261116 (UI polish lane) |
| glyphs | 777 | all-glyphs-live delta — lead/glyph-aa-ease-20261116 |
| pan | 111 | pan L50..C..R50 delta — lead/pan-clip-50-20261116 (1d02a091) |
| drumpara | 91 | R22 909/808 voice page — lead/drum-para-r22-20261116 |
| body | 777 | R22 body/layout gap vs target — owner per SCREEN-INVENTORY row (lead UI polish batches 2–4) |

## Entries

| state | status | target (R22) | px ≠ target | deviations |
|---|---|---|---:|---|
| [`909-seq-00`](909-seq-00/) | KNOWN-DEVIATION | `source-frame-066` | 40662 | battery, footer, glyphs, body |
| [`909-seq-00-touch`](909-seq-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`909-seq-00-voice-01`](909-seq-00-voice-01/) | KNOWN-DEVIATION | `source-frame-066` | 40749 | battery, footer, glyphs, body |
| [`909-seq-00-voice-02`](909-seq-00-voice-02/) | KNOWN-DEVIATION | `source-frame-066` | 40789 | battery, footer, glyphs, body |
| [`909-seq-00-voice-03`](909-seq-00-voice-03/) | KNOWN-DEVIATION | `source-frame-066` | 40762 | battery, footer, glyphs, body |
| [`909-seq-00-voice-04`](909-seq-00-voice-04/) | KNOWN-DEVIATION | `source-frame-066` | 40780 | battery, footer, glyphs, body |
| [`909-seq-00-voice-05`](909-seq-00-voice-05/) | KNOWN-DEVIATION | `source-frame-066` | 40761 | battery, footer, glyphs, body |
| [`909-seq-00-voice-06`](909-seq-00-voice-06/) | KNOWN-DEVIATION | `source-frame-066` | 40776 | battery, footer, glyphs, body |
| [`909-seq-00-voice-07`](909-seq-00-voice-07/) | KNOWN-DEVIATION | `source-frame-066` | 40756 | battery, footer, glyphs, body |
| [`909-seq-00-voice-08`](909-seq-00-voice-08/) | KNOWN-DEVIATION | `source-frame-066` | 40746 | battery, footer, glyphs, body |
| [`909-seq-00-voice-09`](909-seq-00-voice-09/) | KNOWN-DEVIATION | `source-frame-068` | 40731 | battery, footer, glyphs, body |
| [`909-seq-00-voice-10`](909-seq-00-voice-10/) | KNOWN-DEVIATION | `source-frame-066` | 40758 | battery, footer, glyphs, body |
| [`909-seq-00-held`](909-seq-00-held/) | KNOWN-DEVIATION | `source-frame-066` | 40678 | battery, footer, glyphs, body |
| [`909-seq-list-00`](909-seq-list-00/) | KNOWN-DEVIATION | `source-frame-067` | 21871 | battery, footer, glyphs, body |
| [`909-seq-list-01`](909-seq-list-01/) | KNOWN-DEVIATION | `source-frame-069` | 26433 | battery, footer, glyphs, body |
| [`909-para-00`](909-para-00/) | KNOWN-DEVIATION | `909-voice-00` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-00-touch`](909-para-00-touch/) | KNOWN-DEVIATION | `touch-master-2-deadline` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-00-voice-01`](909-para-00-voice-01/) | KNOWN-DEVIATION | `909-voice-01` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-00-voice-02`](909-para-00-voice-02/) | KNOWN-DEVIATION | `909-voice-02` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-00-voice-03`](909-para-00-voice-03/) | KNOWN-DEVIATION | `909-voice-03` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-00-voice-04`](909-para-00-voice-04/) | KNOWN-DEVIATION | `909-voice-04` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-00-voice-05`](909-para-00-voice-05/) | KNOWN-DEVIATION | `909-voice-05` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-00-voice-06`](909-para-00-voice-06/) | KNOWN-DEVIATION | `909-voice-06` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-00-voice-07`](909-para-00-voice-07/) | KNOWN-DEVIATION | `909-voice-07` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-00-voice-08`](909-para-00-voice-08/) | KNOWN-DEVIATION | `909-voice-08` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-00-voice-09`](909-para-00-voice-09/) | KNOWN-DEVIATION | `909-voice-09` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-00-voice-10`](909-para-00-voice-10/) | KNOWN-DEVIATION | `909-voice-10` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-01`](909-para-01/) | KNOWN-DEVIATION | `source-frame-003` | 19214 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-01-touch`](909-para-01-touch/) | KNOWN-DEVIATION | `touch-master-0-before` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-01-voice-01`](909-para-01-voice-01/) | KNOWN-DEVIATION | `source-frame-008` | 17556 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-01-voice-02`](909-para-01-voice-02/) | KNOWN-DEVIATION | `source-frame-008` | 17122 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-01-voice-03`](909-para-01-voice-03/) | KNOWN-DEVIATION | `source-frame-008` | 17222 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-01-voice-04`](909-para-01-voice-04/) | KNOWN-DEVIATION | `source-frame-008` | 17219 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-01-voice-05`](909-para-01-voice-05/) | KNOWN-DEVIATION | `source-frame-008` | 16644 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-01-voice-06`](909-para-01-voice-06/) | KNOWN-DEVIATION | `source-frame-008` | 16874 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-01-voice-07`](909-para-01-voice-07/) | KNOWN-DEVIATION | `source-frame-008` | 16872 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-01-voice-08`](909-para-01-voice-08/) | KNOWN-DEVIATION | `source-frame-008` | 16869 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-01-voice-09`](909-para-01-voice-09/) | KNOWN-DEVIATION | `source-frame-008` | 16774 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-01-voice-10`](909-para-01-voice-10/) | KNOWN-DEVIATION | `source-frame-008` | 16774 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-02`](909-para-02/) | KNOWN-DEVIATION | `source-frame-004` | 19227 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-02-touch`](909-para-02-touch/) | KNOWN-DEVIATION | `touch-master-0-before` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-02-voice-01`](909-para-02-voice-01/) | KNOWN-DEVIATION | `source-frame-007` | 17354 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-02-voice-02`](909-para-02-voice-02/) | KNOWN-DEVIATION | `source-frame-007` | 16978 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-02-voice-03`](909-para-02-voice-03/) | KNOWN-DEVIATION | `source-frame-007` | 17054 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-02-voice-04`](909-para-02-voice-04/) | KNOWN-DEVIATION | `source-frame-007` | 17049 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-02-voice-05`](909-para-02-voice-05/) | KNOWN-DEVIATION | `source-frame-007` | 16393 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-02-voice-06`](909-para-02-voice-06/) | KNOWN-DEVIATION | `source-frame-007` | 16682 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-02-voice-07`](909-para-02-voice-07/) | KNOWN-DEVIATION | `source-frame-007` | 16704 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-02-voice-08`](909-para-02-voice-08/) | KNOWN-DEVIATION | `source-frame-007` | 16700 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-02-voice-09`](909-para-02-voice-09/) | KNOWN-DEVIATION | `source-frame-007` | 16632 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-02-voice-10`](909-para-02-voice-10/) | KNOWN-DEVIATION | `source-frame-007` | 16632 | battery, footer, glyphs, pan, drumpara, body |
| [`909-para-list-00`](909-para-list-00/) | KNOWN-DEVIATION | `source-frame-005` | 25839 | battery, glyphs, pan, drumpara, body |
| [`909-para-list-01`](909-para-list-01/) | KNOWN-DEVIATION | `source-frame-005` | 30263 | battery, glyphs, pan, drumpara, body |
| [`909-para-list-02`](909-para-list-02/) | KNOWN-DEVIATION | `source-frame-005` | 30350 | battery, glyphs, pan, drumpara, body |
| [`909-para-list-03`](909-para-list-03/) | KNOWN-DEVIATION | `source-frame-005` | 30383 | battery, glyphs, pan, drumpara, body |
| [`909-para-list-04`](909-para-list-04/) | KNOWN-DEVIATION | `source-frame-005` | 30488 | battery, glyphs, pan, drumpara, body |
| [`909-para-list-05`](909-para-list-05/) | KNOWN-DEVIATION | `source-frame-018` | 33611 | battery, glyphs, pan, drumpara, body |
| [`909-para-list-06`](909-para-list-06/) | KNOWN-DEVIATION | `source-frame-019` | 35867 | battery, glyphs, pan, drumpara, body |
| [`909-para-list-07`](909-para-list-07/) | KNOWN-DEVIATION | `source-frame-019` | 36112 | battery, glyphs, pan, drumpara, body |
| [`909-para-list-08`](909-para-list-08/) | KNOWN-DEVIATION | `source-frame-019` | 36112 | battery, glyphs, pan, drumpara, body |
| [`909-para-list-09`](909-para-list-09/) | KNOWN-DEVIATION | `source-frame-019` | 36112 | battery, glyphs, pan, drumpara, body |
| [`909-para-list-10`](909-para-list-10/) | KNOWN-DEVIATION | `source-frame-019` | 36112 | battery, glyphs, pan, drumpara, body |
| [`909-fx-00`](909-fx-00/) | KNOWN-DEVIATION | `fx-reverb-direct` | 57600 | battery, footer, glyphs, body |
| [`909-fx-00-touch`](909-fx-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`909-fx-01`](909-fx-01/) | KNOWN-DEVIATION | `fx-delay-direct` | 57600 | battery, footer, glyphs, body |
| [`909-fx-01-touch`](909-fx-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`909-fx-list-00`](909-fx-list-00/) | KNOWN-DEVIATION | `source-frame-046` | 34900 | battery, footer, glyphs, body |
| [`909-fx-list-01`](909-fx-list-01/) | KNOWN-DEVIATION | `source-frame-046` | 39360 | battery, footer, glyphs, body |
| [`909-fx-list-02`](909-fx-list-02/) | KNOWN-DEVIATION | `source-frame-046` | 39751 | battery, footer, glyphs, body |
| [`909-fx-list-03`](909-fx-list-03/) | KNOWN-DEVIATION | `source-frame-046` | 38935 | battery, footer, glyphs, body |
| [`909-fx-list-04`](909-fx-list-04/) | KNOWN-DEVIATION | `source-frame-046` | 39362 | battery, footer, glyphs, body |
| [`909-fx-list-05`](909-fx-list-05/) | KNOWN-DEVIATION | `source-frame-046` | 39401 | battery, footer, glyphs, body |
| [`909-fx-list-06`](909-fx-list-06/) | KNOWN-DEVIATION | `source-frame-055` | 38000 | battery, footer, glyphs, body |
| [`909-fx-list-07`](909-fx-list-07/) | KNOWN-DEVIATION | `source-frame-084` | 38741 | battery, footer, glyphs, body |
| [`909-fx-list-08`](909-fx-list-08/) | KNOWN-DEVIATION | `source-frame-046` | 42831 | battery, footer, glyphs, body |
| [`909-fx-list-09`](909-fx-list-09/) | KNOWN-DEVIATION | `source-frame-053` | 42355 | battery, footer, glyphs, body |
| [`909-fx-list-10`](909-fx-list-10/) | KNOWN-DEVIATION | `source-frame-047` | 40437 | battery, footer, glyphs, body |
| [`909-fx-list-11`](909-fx-list-11/) | KNOWN-DEVIATION | `source-frame-047` | 40437 | battery, footer, glyphs, body |
| [`909-mix-00`](909-mix-00/) | KNOWN-DEVIATION | `mix-parts-live` | 57600 | battery, footer, glyphs, pan, body |
| [`909-mix-00-touch`](909-mix-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`909-mix-01`](909-mix-01/) | KNOWN-DEVIATION | `mix-909-00` | 57600 | battery, footer, glyphs, pan, body |
| [`909-mix-01-touch`](909-mix-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`909-mix-02`](909-mix-02/) | KNOWN-DEVIATION | `mix-808-00` | 57600 | battery, footer, glyphs, pan, body |
| [`909-mix-02-touch`](909-mix-02-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`909-mix-03`](909-mix-03/) | KNOWN-DEVIATION | `mix-master` | 57544 | battery, footer, glyphs, body |
| [`909-mix-03-touch`](909-mix-03-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`909-mix-04`](909-mix-04/) | KNOWN-DEVIATION | `mix-wave` | 57600 | battery, footer, glyphs, body |
| [`909-mix-04-touch`](909-mix-04-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`909-mix-list-00`](909-mix-list-00/) | KNOWN-DEVIATION | `source-frame-053` | 34813 | battery, footer, glyphs, body |
| [`909-mix-list-01`](909-mix-list-01/) | KNOWN-DEVIATION | `source-frame-053` | 39512 | battery, footer, glyphs, body |
| [`909-mix-list-02`](909-mix-list-02/) | KNOWN-DEVIATION | `source-frame-053` | 39389 | battery, footer, glyphs, body |
| [`909-mix-list-03`](909-mix-list-03/) | KNOWN-DEVIATION | `source-frame-053` | 39415 | battery, footer, glyphs, body |
| [`909-mix-list-04`](909-mix-list-04/) | KNOWN-DEVIATION | `source-frame-053` | 38476 | battery, footer, glyphs, body |
| [`909-mix-list-05`](909-mix-list-05/) | KNOWN-DEVIATION | `source-frame-053` | 38966 | battery, footer, glyphs, body |
| [`909-mix-list-06`](909-mix-list-06/) | KNOWN-DEVIATION | `source-frame-053` | 39467 | battery, footer, glyphs, body |
| [`909-mix-list-07`](909-mix-list-07/) | KNOWN-DEVIATION | `source-frame-054` | 40627 | battery, footer, glyphs, body |
| [`909-mix-list-08`](909-mix-list-08/) | KNOWN-DEVIATION | `source-frame-054` | 40858 | battery, footer, glyphs, body |
| [`909-mix-list-09`](909-mix-list-09/) | KNOWN-DEVIATION | `source-frame-053` | 39814 | battery, footer, glyphs, body |
| [`909-mix-list-10`](909-mix-list-10/) | KNOWN-DEVIATION | `source-frame-054` | 40863 | battery, footer, glyphs, body |
| [`909-mix-list-11`](909-mix-list-11/) | KNOWN-DEVIATION | `source-frame-054` | 40621 | battery, footer, glyphs, body |
| [`909-mix-list-12`](909-mix-list-12/) | KNOWN-DEVIATION | `source-frame-054` | 40877 | battery, footer, glyphs, body |
| [`909-mix-list-13`](909-mix-list-13/) | KNOWN-DEVIATION | `source-frame-062` | 39743 | battery, footer, glyphs, body |
| [`909-mix-list-14`](909-mix-list-14/) | KNOWN-DEVIATION | `source-frame-062` | 39632 | battery, footer, glyphs, body |
| [`909-mix-list-15`](909-mix-list-15/) | KNOWN-DEVIATION | `source-frame-062` | 39507 | battery, footer, glyphs, body |
| [`909-mix-list-16`](909-mix-list-16/) | KNOWN-DEVIATION | `source-frame-055` | 38025 | battery, footer, glyphs, body |
| [`909-mix-list-17`](909-mix-list-17/) | KNOWN-DEVIATION | `source-frame-084` | 38712 | battery, footer, glyphs, body |
| [`909-mix-list-18`](909-mix-list-18/) | KNOWN-DEVIATION | `source-frame-055` | 44012 | battery, footer, glyphs, body |
| [`909-mix-list-19`](909-mix-list-19/) | KNOWN-DEVIATION | `source-frame-053` | 42200 | battery, footer, glyphs, body |
| [`909-mix-list-20`](909-mix-list-20/) | KNOWN-DEVIATION | `source-frame-056` | 40032 | battery, footer, glyphs, body |
| [`909-mix-list-21`](909-mix-list-21/) | KNOWN-DEVIATION | `source-frame-056` | 40068 | battery, footer, glyphs, body |
| [`909-mix-list-22`](909-mix-list-22/) | KNOWN-DEVIATION | `source-frame-056` | 39835 | battery, footer, glyphs, body |
| [`909-mix-list-23`](909-mix-list-23/) | KNOWN-DEVIATION | `source-frame-056` | 40063 | battery, footer, glyphs, body |
| [`909-mix-list-24`](909-mix-list-24/) | KNOWN-DEVIATION | `source-frame-053` | 39576 | battery, footer, glyphs, body |
| [`909-mix-list-25`](909-mix-list-25/) | KNOWN-DEVIATION | `source-frame-038` | 39229 | battery, footer, glyphs, body |
| [`909-mix-list-26`](909-mix-list-26/) | KNOWN-DEVIATION | `source-frame-038` | 39040 | battery, footer, glyphs, body |
| [`909-mix-list-27`](909-mix-list-27/) | KNOWN-DEVIATION | `source-frame-062` | 39154 | battery, footer, glyphs, body |
| [`909-mix-list-28`](909-mix-list-28/) | KNOWN-DEVIATION | `source-frame-062` | 39154 | battery, footer, glyphs, body |
| [`909-home-00`](909-home-00/) | KNOWN-DEVIATION | `home-01-16` | 57600 | battery, footer, glyphs, body |
| [`909-home-00-touch`](909-home-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`909-home-list-00`](909-home-list-00/) | KNOWN-DEVIATION | `source-frame-001` | 29792 | battery, footer, glyphs, body |
| [`909-home-list-01`](909-home-list-01/) | KNOWN-DEVIATION | `source-frame-001` | 33831 | battery, footer, glyphs, body |
| [`909-home-list-02`](909-home-list-02/) | KNOWN-DEVIATION | `source-frame-001` | 33900 | battery, footer, glyphs, body |
| [`909-home-list-03`](909-home-list-03/) | KNOWN-DEVIATION | `source-frame-001` | 33857 | battery, footer, glyphs, body |
| [`808-seq-00`](808-seq-00/) | KNOWN-DEVIATION | `source-frame-068` | 40639 | battery, footer, glyphs, body |
| [`808-seq-00-touch`](808-seq-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`808-seq-00-voice-01`](808-seq-00-voice-01/) | KNOWN-DEVIATION | `source-frame-068` | 40730 | battery, footer, glyphs, body |
| [`808-seq-00-voice-02`](808-seq-00-voice-02/) | KNOWN-DEVIATION | `source-frame-068` | 40769 | battery, footer, glyphs, body |
| [`808-seq-00-voice-03`](808-seq-00-voice-03/) | KNOWN-DEVIATION | `source-frame-068` | 40742 | battery, footer, glyphs, body |
| [`808-seq-00-voice-04`](808-seq-00-voice-04/) | KNOWN-DEVIATION | `source-frame-068` | 40760 | battery, footer, glyphs, body |
| [`808-seq-00-voice-05`](808-seq-00-voice-05/) | KNOWN-DEVIATION | `source-frame-068` | 40741 | battery, footer, glyphs, body |
| [`808-seq-00-voice-06`](808-seq-00-voice-06/) | KNOWN-DEVIATION | `source-frame-068` | 40756 | battery, footer, glyphs, body |
| [`808-seq-00-voice-07`](808-seq-00-voice-07/) | KNOWN-DEVIATION | `source-frame-068` | 40745 | battery, footer, glyphs, body |
| [`808-seq-00-voice-08`](808-seq-00-voice-08/) | KNOWN-DEVIATION | `source-frame-068` | 40753 | battery, footer, glyphs, body |
| [`808-seq-00-voice-09`](808-seq-00-voice-09/) | KNOWN-DEVIATION | `source-frame-068` | 40725 | battery, footer, glyphs, body |
| [`808-seq-00-voice-10`](808-seq-00-voice-10/) | KNOWN-DEVIATION | `source-frame-068` | 40735 | battery, footer, glyphs, body |
| [`808-seq-00-held`](808-seq-00-held/) | KNOWN-DEVIATION | `source-frame-068` | 40655 | battery, footer, glyphs, body |
| [`808-seq-list-00`](808-seq-list-00/) | KNOWN-DEVIATION | `source-frame-069` | 22012 | battery, footer, glyphs, body |
| [`808-seq-list-01`](808-seq-list-01/) | KNOWN-DEVIATION | `source-frame-069` | 26431 | battery, footer, glyphs, body |
| [`808-para-00`](808-para-00/) | KNOWN-DEVIATION | `808-voice-00` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-00-touch`](808-para-00-touch/) | KNOWN-DEVIATION | `touch-fx-delay-0-deadline` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-00-voice-01`](808-para-00-voice-01/) | KNOWN-DEVIATION | `808-voice-01` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-00-voice-02`](808-para-00-voice-02/) | KNOWN-DEVIATION | `808-voice-02` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-00-voice-03`](808-para-00-voice-03/) | KNOWN-DEVIATION | `808-voice-03` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-00-voice-04`](808-para-00-voice-04/) | KNOWN-DEVIATION | `808-voice-04` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-00-voice-05`](808-para-00-voice-05/) | KNOWN-DEVIATION | `808-voice-05` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-00-voice-06`](808-para-00-voice-06/) | KNOWN-DEVIATION | `808-voice-06` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-00-voice-07`](808-para-00-voice-07/) | KNOWN-DEVIATION | `808-voice-07` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-00-voice-08`](808-para-00-voice-08/) | KNOWN-DEVIATION | `808-voice-08` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-00-voice-09`](808-para-00-voice-09/) | KNOWN-DEVIATION | `808-voice-09` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-00-voice-10`](808-para-00-voice-10/) | KNOWN-DEVIATION | `808-voice-10` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-01`](808-para-01/) | KNOWN-DEVIATION | `source-frame-008` | 17162 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-01-touch`](808-para-01-touch/) | KNOWN-DEVIATION | `touch-fx-delay-0-deadline` | 57600 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-01-voice-01`](808-para-01-voice-01/) | KNOWN-DEVIATION | `source-frame-008` | 17221 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-01-voice-02`](808-para-01-voice-02/) | KNOWN-DEVIATION | `source-frame-008` | 17212 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-01-voice-03`](808-para-01-voice-03/) | KNOWN-DEVIATION | `source-frame-008` | 17215 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-01-voice-04`](808-para-01-voice-04/) | KNOWN-DEVIATION | `source-frame-008` | 17214 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-01-voice-05`](808-para-01-voice-05/) | KNOWN-DEVIATION | `source-frame-008` | 17095 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-01-voice-06`](808-para-01-voice-06/) | KNOWN-DEVIATION | `source-frame-008` | 17392 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-01-voice-07`](808-para-01-voice-07/) | KNOWN-DEVIATION | `source-frame-008` | 16768 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-01-voice-08`](808-para-01-voice-08/) | KNOWN-DEVIATION | `source-frame-008` | 16769 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-01-voice-09`](808-para-01-voice-09/) | KNOWN-DEVIATION | `source-frame-008` | 16773 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-01-voice-10`](808-para-01-voice-10/) | KNOWN-DEVIATION | `source-frame-008` | 16765 | battery, footer, glyphs, pan, drumpara, body |
| [`808-para-list-00`](808-para-list-00/) | KNOWN-DEVIATION | `source-frame-009` | 25440 | battery, glyphs, pan, drumpara, body |
| [`808-para-list-01`](808-para-list-01/) | KNOWN-DEVIATION | `source-frame-009` | 29631 | battery, glyphs, pan, drumpara, body |
| [`808-para-list-02`](808-para-list-02/) | KNOWN-DEVIATION | `source-frame-009` | 29475 | battery, glyphs, pan, drumpara, body |
| [`808-para-list-03`](808-para-list-03/) | KNOWN-DEVIATION | `source-frame-009` | 29661 | battery, glyphs, pan, drumpara, body |
| [`808-para-list-04`](808-para-list-04/) | KNOWN-DEVIATION | `source-frame-019` | 33569 | battery, glyphs, pan, drumpara, body |
| [`808-para-list-05`](808-para-list-05/) | KNOWN-DEVIATION | `source-frame-019` | 35885 | battery, glyphs, pan, drumpara, body |
| [`808-para-list-06`](808-para-list-06/) | KNOWN-DEVIATION | `source-frame-019` | 36110 | battery, glyphs, pan, drumpara, body |
| [`808-para-list-07`](808-para-list-07/) | KNOWN-DEVIATION | `source-frame-019` | 36110 | battery, glyphs, pan, drumpara, body |
| [`808-para-list-08`](808-para-list-08/) | KNOWN-DEVIATION | `source-frame-019` | 36110 | battery, glyphs, pan, drumpara, body |
| [`808-para-list-09`](808-para-list-09/) | KNOWN-DEVIATION | `source-frame-019` | 36110 | battery, glyphs, pan, drumpara, body |
| [`808-fx-00`](808-fx-00/) | KNOWN-DEVIATION | `fx-reverb-direct` | 57600 | battery, footer, glyphs, body |
| [`808-fx-00-touch`](808-fx-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`808-fx-01`](808-fx-01/) | KNOWN-DEVIATION | `fx-delay-direct` | 57600 | battery, footer, glyphs, body |
| [`808-fx-01-touch`](808-fx-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`808-fx-list-00`](808-fx-list-00/) | KNOWN-DEVIATION | `source-frame-046` | 36785 | battery, footer, glyphs, body |
| [`808-fx-list-01`](808-fx-list-01/) | KNOWN-DEVIATION | `source-frame-046` | 39360 | battery, footer, glyphs, body |
| [`808-fx-list-02`](808-fx-list-02/) | KNOWN-DEVIATION | `source-frame-046` | 39751 | battery, footer, glyphs, body |
| [`808-fx-list-03`](808-fx-list-03/) | KNOWN-DEVIATION | `source-frame-046` | 38935 | battery, footer, glyphs, body |
| [`808-fx-list-04`](808-fx-list-04/) | KNOWN-DEVIATION | `source-frame-046` | 39362 | battery, footer, glyphs, body |
| [`808-fx-list-05`](808-fx-list-05/) | KNOWN-DEVIATION | `source-frame-046` | 39401 | battery, footer, glyphs, body |
| [`808-fx-list-06`](808-fx-list-06/) | KNOWN-DEVIATION | `source-frame-055` | 38000 | battery, footer, glyphs, body |
| [`808-fx-list-07`](808-fx-list-07/) | KNOWN-DEVIATION | `source-frame-084` | 38741 | battery, footer, glyphs, body |
| [`808-fx-list-08`](808-fx-list-08/) | KNOWN-DEVIATION | `source-frame-046` | 42831 | battery, footer, glyphs, body |
| [`808-fx-list-09`](808-fx-list-09/) | KNOWN-DEVIATION | `source-frame-053` | 42355 | battery, footer, glyphs, body |
| [`808-fx-list-10`](808-fx-list-10/) | KNOWN-DEVIATION | `source-frame-047` | 40437 | battery, footer, glyphs, body |
| [`808-fx-list-11`](808-fx-list-11/) | KNOWN-DEVIATION | `source-frame-047` | 40437 | battery, footer, glyphs, body |
| [`808-mix-00`](808-mix-00/) | KNOWN-DEVIATION | `mix-parts-live` | 57600 | battery, footer, glyphs, pan, body |
| [`808-mix-00-touch`](808-mix-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`808-mix-01`](808-mix-01/) | KNOWN-DEVIATION | `mix-909-00` | 57600 | battery, footer, glyphs, pan, body |
| [`808-mix-01-touch`](808-mix-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`808-mix-02`](808-mix-02/) | KNOWN-DEVIATION | `mix-808-00` | 57600 | battery, footer, glyphs, pan, body |
| [`808-mix-02-touch`](808-mix-02-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`808-mix-03`](808-mix-03/) | KNOWN-DEVIATION | `mix-master` | 57600 | battery, footer, glyphs, body |
| [`808-mix-03-touch`](808-mix-03-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`808-mix-04`](808-mix-04/) | KNOWN-DEVIATION | `mix-wave` | 57600 | battery, footer, glyphs, body |
| [`808-mix-04-touch`](808-mix-04-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`808-mix-list-00`](808-mix-list-00/) | KNOWN-DEVIATION | `source-frame-053` | 36628 | battery, footer, glyphs, body |
| [`808-mix-list-01`](808-mix-list-01/) | KNOWN-DEVIATION | `source-frame-053` | 39512 | battery, footer, glyphs, body |
| [`808-mix-list-02`](808-mix-list-02/) | KNOWN-DEVIATION | `source-frame-053` | 39389 | battery, footer, glyphs, body |
| [`808-mix-list-03`](808-mix-list-03/) | KNOWN-DEVIATION | `source-frame-053` | 39415 | battery, footer, glyphs, body |
| [`808-mix-list-04`](808-mix-list-04/) | KNOWN-DEVIATION | `source-frame-053` | 38476 | battery, footer, glyphs, body |
| [`808-mix-list-05`](808-mix-list-05/) | KNOWN-DEVIATION | `source-frame-053` | 38966 | battery, footer, glyphs, body |
| [`808-mix-list-06`](808-mix-list-06/) | KNOWN-DEVIATION | `source-frame-053` | 39467 | battery, footer, glyphs, body |
| [`808-mix-list-07`](808-mix-list-07/) | KNOWN-DEVIATION | `source-frame-054` | 40627 | battery, footer, glyphs, body |
| [`808-mix-list-08`](808-mix-list-08/) | KNOWN-DEVIATION | `source-frame-054` | 40858 | battery, footer, glyphs, body |
| [`808-mix-list-09`](808-mix-list-09/) | KNOWN-DEVIATION | `source-frame-053` | 39814 | battery, footer, glyphs, body |
| [`808-mix-list-10`](808-mix-list-10/) | KNOWN-DEVIATION | `source-frame-054` | 40863 | battery, footer, glyphs, body |
| [`808-mix-list-11`](808-mix-list-11/) | KNOWN-DEVIATION | `source-frame-054` | 40621 | battery, footer, glyphs, body |
| [`808-mix-list-12`](808-mix-list-12/) | KNOWN-DEVIATION | `source-frame-054` | 40877 | battery, footer, glyphs, body |
| [`808-mix-list-13`](808-mix-list-13/) | KNOWN-DEVIATION | `source-frame-062` | 39743 | battery, footer, glyphs, body |
| [`808-mix-list-14`](808-mix-list-14/) | KNOWN-DEVIATION | `source-frame-062` | 39632 | battery, footer, glyphs, body |
| [`808-mix-list-15`](808-mix-list-15/) | KNOWN-DEVIATION | `source-frame-062` | 39507 | battery, footer, glyphs, body |
| [`808-mix-list-16`](808-mix-list-16/) | KNOWN-DEVIATION | `source-frame-055` | 38025 | battery, footer, glyphs, body |
| [`808-mix-list-17`](808-mix-list-17/) | KNOWN-DEVIATION | `source-frame-084` | 38712 | battery, footer, glyphs, body |
| [`808-mix-list-18`](808-mix-list-18/) | KNOWN-DEVIATION | `source-frame-055` | 44012 | battery, footer, glyphs, body |
| [`808-mix-list-19`](808-mix-list-19/) | KNOWN-DEVIATION | `source-frame-053` | 42200 | battery, footer, glyphs, body |
| [`808-mix-list-20`](808-mix-list-20/) | KNOWN-DEVIATION | `source-frame-056` | 40032 | battery, footer, glyphs, body |
| [`808-mix-list-21`](808-mix-list-21/) | KNOWN-DEVIATION | `source-frame-056` | 40068 | battery, footer, glyphs, body |
| [`808-mix-list-22`](808-mix-list-22/) | KNOWN-DEVIATION | `source-frame-056` | 39835 | battery, footer, glyphs, body |
| [`808-mix-list-23`](808-mix-list-23/) | KNOWN-DEVIATION | `source-frame-056` | 40063 | battery, footer, glyphs, body |
| [`808-mix-list-24`](808-mix-list-24/) | KNOWN-DEVIATION | `source-frame-053` | 39576 | battery, footer, glyphs, body |
| [`808-mix-list-25`](808-mix-list-25/) | KNOWN-DEVIATION | `source-frame-038` | 39229 | battery, footer, glyphs, body |
| [`808-mix-list-26`](808-mix-list-26/) | KNOWN-DEVIATION | `source-frame-038` | 39040 | battery, footer, glyphs, body |
| [`808-mix-list-27`](808-mix-list-27/) | KNOWN-DEVIATION | `source-frame-062` | 39154 | battery, footer, glyphs, body |
| [`808-mix-list-28`](808-mix-list-28/) | KNOWN-DEVIATION | `source-frame-062` | 39154 | battery, footer, glyphs, body |
| [`808-home-00`](808-home-00/) | KNOWN-DEVIATION | `home-01-16` | 57600 | battery, footer, glyphs, body |
| [`808-home-00-touch`](808-home-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`808-home-list-00`](808-home-list-00/) | KNOWN-DEVIATION | `source-frame-001` | 31372 | battery, footer, glyphs, body |
| [`808-home-list-01`](808-home-list-01/) | KNOWN-DEVIATION | `source-frame-001` | 33831 | battery, footer, glyphs, body |
| [`808-home-list-02`](808-home-list-02/) | KNOWN-DEVIATION | `source-frame-001` | 33900 | battery, footer, glyphs, body |
| [`808-home-list-03`](808-home-list-03/) | KNOWN-DEVIATION | `source-frame-001` | 33857 | battery, footer, glyphs, body |
| [`break-seq-00`](break-seq-00/) | KNOWN-DEVIATION | `retained-family-10` | 53566 | battery, footer, glyphs, body |
| [`break-seq-00-touch`](break-seq-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`break-seq-00-held`](break-seq-00-held/) | KNOWN-DEVIATION | `retained-family-10` | 53648 | battery, footer, glyphs, body |
| [`break-seq-01`](break-seq-01/) | KNOWN-DEVIATION | `retained-family-10` | 53566 | battery, footer, glyphs, body |
| [`break-seq-01-touch`](break-seq-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`break-seq-01-held`](break-seq-01-held/) | KNOWN-DEVIATION | `retained-family-10` | 53648 | battery, footer, glyphs, body |
| [`break-seq-02`](break-seq-02/) | KNOWN-DEVIATION | `retained-family-10` | 53532 | battery, footer, glyphs, body |
| [`break-seq-02-touch`](break-seq-02-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57598 | battery, footer, glyphs, body |
| [`break-seq-02-held`](break-seq-02-held/) | KNOWN-DEVIATION | `retained-family-10` | 53614 | battery, footer, glyphs, body |
| [`break-seq-list-00`](break-seq-list-00/) | KNOWN-DEVIATION | `source-frame-083` | 35549 | battery, footer, glyphs, body |
| [`break-seq-list-01`](break-seq-list-01/) | KNOWN-DEVIATION | `source-frame-083` | 40088 | battery, footer, glyphs, body |
| [`break-seq-list-02`](break-seq-list-02/) | KNOWN-DEVIATION | `source-frame-083` | 39962 | battery, footer, glyphs, body |
| [`break-seq-list-03`](break-seq-list-03/) | KNOWN-DEVIATION | `source-frame-083` | 40216 | battery, footer, glyphs, body |
| [`break-seq-list-04`](break-seq-list-04/) | KNOWN-DEVIATION | `source-frame-083` | 40281 | battery, footer, glyphs, body |
| [`break-seq-list-05`](break-seq-list-05/) | KNOWN-DEVIATION | `source-frame-083` | 40110 | battery, footer, glyphs, body |
| [`break-seq-list-06`](break-seq-list-06/) | KNOWN-DEVIATION | `source-frame-083` | 40316 | battery, footer, glyphs, body |
| [`break-seq-list-07`](break-seq-list-07/) | KNOWN-DEVIATION | `source-frame-083` | 40308 | battery, footer, glyphs, body |
| [`break-seq-list-08`](break-seq-list-08/) | KNOWN-DEVIATION | `source-frame-081` | 40512 | battery, footer, glyphs, body |
| [`break-seq-list-09`](break-seq-list-09/) | KNOWN-DEVIATION | `source-frame-055` | 38750 | battery, footer, glyphs, body |
| [`break-seq-list-10`](break-seq-list-10/) | KNOWN-DEVIATION | `source-frame-084` | 38570 | battery, footer, glyphs, body |
| [`break-seq-list-11`](break-seq-list-11/) | KNOWN-DEVIATION | `source-frame-081` | 42789 | battery, footer, glyphs, body |
| [`break-seq-list-12`](break-seq-list-12/) | KNOWN-DEVIATION | `source-frame-081` | 42789 | battery, footer, glyphs, body |
| [`break-para-00`](break-para-00/) | KNOWN-DEVIATION | `retained-family-10` | 53568 | battery, footer, glyphs, body |
| [`break-para-00-touch`](break-para-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`break-para-list-00`](break-para-list-00/) | KNOWN-DEVIATION | `source-frame-034` | 33940 | battery, footer, glyphs, body |
| [`break-para-list-01`](break-para-list-01/) | KNOWN-DEVIATION | `source-frame-034` | 38179 | battery, footer, glyphs, body |
| [`break-para-list-02`](break-para-list-02/) | KNOWN-DEVIATION | `source-frame-034` | 38060 | battery, footer, glyphs, body |
| [`break-para-list-03`](break-para-list-03/) | KNOWN-DEVIATION | `source-frame-034` | 38648 | battery, footer, glyphs, body |
| [`break-para-list-04`](break-para-list-04/) | KNOWN-DEVIATION | `source-frame-034` | 38161 | battery, footer, glyphs, body |
| [`break-para-list-05`](break-para-list-05/) | KNOWN-DEVIATION | `source-frame-084` | 39425 | battery, footer, glyphs, body |
| [`break-para-list-06`](break-para-list-06/) | KNOWN-DEVIATION | `source-frame-038` | 42266 | battery, footer, glyphs, body |
| [`break-para-list-07`](break-para-list-07/) | KNOWN-DEVIATION | `source-frame-038` | 42266 | battery, footer, glyphs, body |
| [`break-fx-00`](break-fx-00/) | KNOWN-DEVIATION | `fx-reverb-direct` | 57600 | battery, footer, glyphs, body |
| [`break-fx-00-touch`](break-fx-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`break-fx-01`](break-fx-01/) | KNOWN-DEVIATION | `fx-delay-direct` | 57600 | battery, footer, glyphs, body |
| [`break-fx-01-touch`](break-fx-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`break-fx-list-00`](break-fx-list-00/) | KNOWN-DEVIATION | `source-frame-046` | 36785 | battery, footer, glyphs, body |
| [`break-fx-list-01`](break-fx-list-01/) | KNOWN-DEVIATION | `source-frame-046` | 39360 | battery, footer, glyphs, body |
| [`break-fx-list-02`](break-fx-list-02/) | KNOWN-DEVIATION | `source-frame-046` | 39751 | battery, footer, glyphs, body |
| [`break-fx-list-03`](break-fx-list-03/) | KNOWN-DEVIATION | `source-frame-046` | 38935 | battery, footer, glyphs, body |
| [`break-fx-list-04`](break-fx-list-04/) | KNOWN-DEVIATION | `source-frame-046` | 39362 | battery, footer, glyphs, body |
| [`break-fx-list-05`](break-fx-list-05/) | KNOWN-DEVIATION | `source-frame-046` | 39401 | battery, footer, glyphs, body |
| [`break-fx-list-06`](break-fx-list-06/) | KNOWN-DEVIATION | `source-frame-055` | 38000 | battery, footer, glyphs, body |
| [`break-fx-list-07`](break-fx-list-07/) | KNOWN-DEVIATION | `source-frame-084` | 38741 | battery, footer, glyphs, body |
| [`break-fx-list-08`](break-fx-list-08/) | KNOWN-DEVIATION | `source-frame-046` | 42831 | battery, footer, glyphs, body |
| [`break-fx-list-09`](break-fx-list-09/) | KNOWN-DEVIATION | `source-frame-053` | 42355 | battery, footer, glyphs, body |
| [`break-fx-list-10`](break-fx-list-10/) | KNOWN-DEVIATION | `source-frame-047` | 40437 | battery, footer, glyphs, body |
| [`break-fx-list-11`](break-fx-list-11/) | KNOWN-DEVIATION | `source-frame-047` | 40437 | battery, footer, glyphs, body |
| [`break-mix-00`](break-mix-00/) | KNOWN-DEVIATION | `mix-parts-live` | 57600 | battery, footer, glyphs, pan, body |
| [`break-mix-00-touch`](break-mix-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`break-mix-01`](break-mix-01/) | KNOWN-DEVIATION | `mix-909-00` | 57600 | battery, footer, glyphs, pan, body |
| [`break-mix-01-touch`](break-mix-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`break-mix-02`](break-mix-02/) | KNOWN-DEVIATION | `mix-808-00` | 57600 | battery, footer, glyphs, pan, body |
| [`break-mix-02-touch`](break-mix-02-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`break-mix-03`](break-mix-03/) | KNOWN-DEVIATION | `mix-master` | 57600 | battery, footer, glyphs, body |
| [`break-mix-03-touch`](break-mix-03-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`break-mix-04`](break-mix-04/) | KNOWN-DEVIATION | `mix-wave` | 57600 | battery, footer, glyphs, body |
| [`break-mix-04-touch`](break-mix-04-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`break-mix-list-00`](break-mix-list-00/) | KNOWN-DEVIATION | `source-frame-053` | 36628 | battery, footer, glyphs, body |
| [`break-mix-list-01`](break-mix-list-01/) | KNOWN-DEVIATION | `source-frame-053` | 39512 | battery, footer, glyphs, body |
| [`break-mix-list-02`](break-mix-list-02/) | KNOWN-DEVIATION | `source-frame-053` | 39389 | battery, footer, glyphs, body |
| [`break-mix-list-03`](break-mix-list-03/) | KNOWN-DEVIATION | `source-frame-053` | 39415 | battery, footer, glyphs, body |
| [`break-mix-list-04`](break-mix-list-04/) | KNOWN-DEVIATION | `source-frame-053` | 38476 | battery, footer, glyphs, body |
| [`break-mix-list-05`](break-mix-list-05/) | KNOWN-DEVIATION | `source-frame-053` | 38966 | battery, footer, glyphs, body |
| [`break-mix-list-06`](break-mix-list-06/) | KNOWN-DEVIATION | `source-frame-053` | 39467 | battery, footer, glyphs, body |
| [`break-mix-list-07`](break-mix-list-07/) | KNOWN-DEVIATION | `source-frame-054` | 40627 | battery, footer, glyphs, body |
| [`break-mix-list-08`](break-mix-list-08/) | KNOWN-DEVIATION | `source-frame-054` | 40858 | battery, footer, glyphs, body |
| [`break-mix-list-09`](break-mix-list-09/) | KNOWN-DEVIATION | `source-frame-053` | 39814 | battery, footer, glyphs, body |
| [`break-mix-list-10`](break-mix-list-10/) | KNOWN-DEVIATION | `source-frame-054` | 40863 | battery, footer, glyphs, body |
| [`break-mix-list-11`](break-mix-list-11/) | KNOWN-DEVIATION | `source-frame-054` | 40621 | battery, footer, glyphs, body |
| [`break-mix-list-12`](break-mix-list-12/) | KNOWN-DEVIATION | `source-frame-054` | 40877 | battery, footer, glyphs, body |
| [`break-mix-list-13`](break-mix-list-13/) | KNOWN-DEVIATION | `source-frame-062` | 39743 | battery, footer, glyphs, body |
| [`break-mix-list-14`](break-mix-list-14/) | KNOWN-DEVIATION | `source-frame-062` | 39632 | battery, footer, glyphs, body |
| [`break-mix-list-15`](break-mix-list-15/) | KNOWN-DEVIATION | `source-frame-062` | 39507 | battery, footer, glyphs, body |
| [`break-mix-list-16`](break-mix-list-16/) | KNOWN-DEVIATION | `source-frame-055` | 38025 | battery, footer, glyphs, body |
| [`break-mix-list-17`](break-mix-list-17/) | KNOWN-DEVIATION | `source-frame-084` | 38712 | battery, footer, glyphs, body |
| [`break-mix-list-18`](break-mix-list-18/) | KNOWN-DEVIATION | `source-frame-055` | 44012 | battery, footer, glyphs, body |
| [`break-mix-list-19`](break-mix-list-19/) | KNOWN-DEVIATION | `source-frame-053` | 42200 | battery, footer, glyphs, body |
| [`break-mix-list-20`](break-mix-list-20/) | KNOWN-DEVIATION | `source-frame-056` | 40032 | battery, footer, glyphs, body |
| [`break-mix-list-21`](break-mix-list-21/) | KNOWN-DEVIATION | `source-frame-056` | 40068 | battery, footer, glyphs, body |
| [`break-mix-list-22`](break-mix-list-22/) | KNOWN-DEVIATION | `source-frame-056` | 39835 | battery, footer, glyphs, body |
| [`break-mix-list-23`](break-mix-list-23/) | KNOWN-DEVIATION | `source-frame-056` | 40063 | battery, footer, glyphs, body |
| [`break-mix-list-24`](break-mix-list-24/) | KNOWN-DEVIATION | `source-frame-053` | 39576 | battery, footer, glyphs, body |
| [`break-mix-list-25`](break-mix-list-25/) | KNOWN-DEVIATION | `source-frame-053` | 39406 | battery, footer, glyphs, body |
| [`break-mix-list-26`](break-mix-list-26/) | KNOWN-DEVIATION | `source-frame-038` | 39040 | battery, footer, glyphs, body |
| [`break-mix-list-27`](break-mix-list-27/) | KNOWN-DEVIATION | `source-frame-062` | 39154 | battery, footer, glyphs, body |
| [`break-mix-list-28`](break-mix-list-28/) | KNOWN-DEVIATION | `source-frame-062` | 39154 | battery, footer, glyphs, body |
| [`break-home-00`](break-home-00/) | KNOWN-DEVIATION | `home-01-16` | 57600 | battery, footer, glyphs, body |
| [`break-home-00-touch`](break-home-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`break-home-list-00`](break-home-list-00/) | KNOWN-DEVIATION | `source-frame-001` | 31372 | battery, footer, glyphs, body |
| [`break-home-list-01`](break-home-list-01/) | KNOWN-DEVIATION | `source-frame-001` | 33831 | battery, footer, glyphs, body |
| [`break-home-list-02`](break-home-list-02/) | KNOWN-DEVIATION | `source-frame-001` | 33900 | battery, footer, glyphs, body |
| [`break-home-list-03`](break-home-list-03/) | KNOWN-DEVIATION | `source-frame-001` | 33857 | battery, footer, glyphs, body |
| [`chicago-seq-00`](chicago-seq-00/) | KNOWN-DEVIATION | `source-frame-071` | 21877 | battery, footer, glyphs, body |
| [`chicago-seq-00-touch`](chicago-seq-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57598 | battery, footer, glyphs, body |
| [`chicago-seq-00-held`](chicago-seq-00-held/) | KNOWN-DEVIATION | `selected-held-initial` | 24566 | battery, footer, glyphs, body |
| [`chicago-seq-01`](chicago-seq-01/) | KNOWN-DEVIATION | `source-frame-072` | 21314 | battery, footer, glyphs, body |
| [`chicago-seq-01-touch`](chicago-seq-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`chicago-seq-01-held`](chicago-seq-01-held/) | KNOWN-DEVIATION | `selected-held-initial` | 24566 | battery, footer, glyphs, body |
| [`chicago-seq-02`](chicago-seq-02/) | KNOWN-DEVIATION | `source-frame-075` | 24479 | battery, footer, glyphs, body |
| [`chicago-seq-02-touch`](chicago-seq-02-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`chicago-seq-02-held`](chicago-seq-02-held/) | KNOWN-DEVIATION | `selected-held-initial` | 24566 | battery, footer, glyphs, body |
| [`chicago-seq-list-00`](chicago-seq-list-00/) | KNOWN-DEVIATION | `source-frame-078` | 32066 | battery, footer, glyphs, body |
| [`chicago-seq-list-01`](chicago-seq-list-01/) | KNOWN-DEVIATION | `source-frame-073` | 36844 | battery, footer, glyphs, body |
| [`chicago-seq-list-02`](chicago-seq-list-02/) | KNOWN-DEVIATION | `source-frame-073` | 36390 | battery, footer, glyphs, body |
| [`chicago-seq-list-03`](chicago-seq-list-03/) | KNOWN-DEVIATION | `source-frame-073` | 36469 | battery, footer, glyphs, body |
| [`chicago-seq-list-04`](chicago-seq-list-04/) | KNOWN-DEVIATION | `source-frame-073` | 36178 | battery, footer, glyphs, body |
| [`chicago-seq-list-05`](chicago-seq-list-05/) | KNOWN-DEVIATION | `source-frame-055` | 38672 | battery, footer, glyphs, body |
| [`chicago-seq-list-06`](chicago-seq-list-06/) | KNOWN-DEVIATION | `source-frame-055` | 38672 | battery, footer, glyphs, body |
| [`chicago-seq-list-07`](chicago-seq-list-07/) | KNOWN-DEVIATION | `source-frame-055` | 38672 | battery, footer, glyphs, body |
| [`chicago-para-00`](chicago-para-00/) | KNOWN-DEVIATION | `source-frame-021` | 27010 | battery, footer, glyphs, body |
| [`chicago-para-00-touch`](chicago-para-00-touch/) | KNOWN-DEVIATION | `touch-master-2-deadline` | 57600 | battery, footer, glyphs, body |
| [`chicago-para-01`](chicago-para-01/) | KNOWN-DEVIATION | `source-frame-022` | 26913 | battery, footer, glyphs, body |
| [`chicago-para-01-touch`](chicago-para-01-touch/) | KNOWN-DEVIATION | `touch-master-0-before` | 57600 | battery, footer, glyphs, body |
| [`chicago-para-02`](chicago-para-02/) | KNOWN-DEVIATION | `source-frame-023` | 26936 | battery, footer, glyphs, body |
| [`chicago-para-02-touch`](chicago-para-02-touch/) | KNOWN-DEVIATION | `touch-master-0-before` | 57600 | battery, footer, glyphs, body |
| [`chicago-para-03`](chicago-para-03/) | KNOWN-DEVIATION | `source-frame-024` | 26572 | battery, footer, glyphs, body |
| [`chicago-para-03-touch`](chicago-para-03-touch/) | KNOWN-DEVIATION | `touch-master-1-queued` | 57600 | battery, footer, glyphs, body |
| [`chicago-para-04`](chicago-para-04/) | KNOWN-DEVIATION | `source-frame-025` | 29300 | battery, footer, glyphs, body |
| [`chicago-para-04-touch`](chicago-para-04-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`chicago-para-05`](chicago-para-05/) | KNOWN-DEVIATION | `source-frame-026` | 28844 | battery, footer, glyphs, body |
| [`chicago-para-05-touch`](chicago-para-05-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`chicago-para-06`](chicago-para-06/) | KNOWN-DEVIATION | `source-frame-027` | 29063 | battery, footer, glyphs, body |
| [`chicago-para-06-touch`](chicago-para-06-touch/) | KNOWN-DEVIATION | `touch-master-2-deadline` | 57600 | battery, footer, glyphs, body |
| [`chicago-para-07`](chicago-para-07/) | KNOWN-DEVIATION | `source-frame-028` | 29033 | battery, footer, glyphs, body |
| [`chicago-para-07-touch`](chicago-para-07-touch/) | KNOWN-DEVIATION | `touch-fx-delay-0-deadline` | 57600 | battery, footer, glyphs, body |
| [`chicago-para-list-00`](chicago-para-list-00/) | KNOWN-DEVIATION | `source-frame-029` | 26173 | battery, glyphs, body |
| [`chicago-para-list-01`](chicago-para-list-01/) | KNOWN-DEVIATION | `source-frame-029` | 30660 | battery, glyphs, body |
| [`chicago-para-list-02`](chicago-para-list-02/) | KNOWN-DEVIATION | `source-frame-029` | 30497 | battery, glyphs, body |
| [`chicago-para-list-03`](chicago-para-list-03/) | KNOWN-DEVIATION | `source-frame-029` | 30688 | battery, glyphs, body |
| [`chicago-para-list-04`](chicago-para-list-04/) | KNOWN-DEVIATION | `source-frame-029` | 30701 | battery, glyphs, body |
| [`chicago-para-list-05`](chicago-para-list-05/) | KNOWN-DEVIATION | `source-frame-029` | 30710 | battery, glyphs, body |
| [`chicago-para-list-06`](chicago-para-list-06/) | KNOWN-DEVIATION | `source-frame-029` | 30849 | battery, glyphs, body |
| [`chicago-para-list-07`](chicago-para-list-07/) | KNOWN-DEVIATION | `source-frame-019` | 31182 | battery, glyphs, body |
| [`chicago-para-list-08`](chicago-para-list-08/) | KNOWN-DEVIATION | `source-frame-018` | 31287 | battery, glyphs, body |
| [`chicago-para-list-09`](chicago-para-list-09/) | KNOWN-DEVIATION | `source-frame-018` | 31312 | battery, glyphs, body |
| [`chicago-para-list-10`](chicago-para-list-10/) | KNOWN-DEVIATION | `source-frame-018` | 31303 | battery, glyphs, body |
| [`chicago-para-list-11`](chicago-para-list-11/) | KNOWN-DEVIATION | `source-frame-030` | 31397 | battery, glyphs, body |
| [`chicago-para-list-12`](chicago-para-list-12/) | KNOWN-DEVIATION | `source-frame-018` | 31513 | battery, glyphs, body |
| [`chicago-para-list-13`](chicago-para-list-13/) | KNOWN-DEVIATION | `source-frame-018` | 31582 | battery, glyphs, body |
| [`chicago-para-list-14`](chicago-para-list-14/) | KNOWN-DEVIATION | `source-frame-018` | 31484 | battery, glyphs, body |
| [`chicago-para-list-15`](chicago-para-list-15/) | KNOWN-DEVIATION | `source-frame-018` | 31493 | battery, glyphs, body |
| [`chicago-para-list-16`](chicago-para-list-16/) | KNOWN-DEVIATION | `source-frame-017` | 30782 | battery, glyphs, body |
| [`chicago-para-list-17`](chicago-para-list-17/) | KNOWN-DEVIATION | `source-frame-031` | 30483 | battery, glyphs, body |
| [`chicago-para-list-18`](chicago-para-list-18/) | KNOWN-DEVIATION | `source-frame-019` | 31134 | battery, glyphs, body |
| [`chicago-para-list-19`](chicago-para-list-19/) | KNOWN-DEVIATION | `source-frame-018` | 31304 | battery, glyphs, body |
| [`chicago-para-list-20`](chicago-para-list-20/) | KNOWN-DEVIATION | `source-frame-019` | 31222 | battery, glyphs, body |
| [`chicago-para-list-21`](chicago-para-list-21/) | KNOWN-DEVIATION | `source-frame-019` | 31123 | battery, glyphs, body |
| [`chicago-para-list-22`](chicago-para-list-22/) | KNOWN-DEVIATION | `source-frame-019` | 31547 | battery, glyphs, body |
| [`chicago-fx-00`](chicago-fx-00/) | KNOWN-DEVIATION | `fx-reverb-direct` | 57600 | battery, footer, glyphs, body |
| [`chicago-fx-00-touch`](chicago-fx-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`chicago-fx-01`](chicago-fx-01/) | KNOWN-DEVIATION | `fx-delay-direct` | 57600 | battery, footer, glyphs, body |
| [`chicago-fx-01-touch`](chicago-fx-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`chicago-fx-list-00`](chicago-fx-list-00/) | KNOWN-DEVIATION | `source-frame-046` | 36785 | battery, footer, glyphs, body |
| [`chicago-fx-list-01`](chicago-fx-list-01/) | KNOWN-DEVIATION | `source-frame-078` | 35537 | battery, footer, glyphs, body |
| [`chicago-fx-list-02`](chicago-fx-list-02/) | KNOWN-DEVIATION | `source-frame-046` | 39751 | battery, footer, glyphs, body |
| [`chicago-fx-list-03`](chicago-fx-list-03/) | KNOWN-DEVIATION | `source-frame-046` | 38935 | battery, footer, glyphs, body |
| [`chicago-fx-list-04`](chicago-fx-list-04/) | KNOWN-DEVIATION | `source-frame-046` | 39362 | battery, footer, glyphs, body |
| [`chicago-fx-list-05`](chicago-fx-list-05/) | KNOWN-DEVIATION | `source-frame-046` | 39401 | battery, footer, glyphs, body |
| [`chicago-fx-list-06`](chicago-fx-list-06/) | KNOWN-DEVIATION | `source-frame-055` | 38000 | battery, footer, glyphs, body |
| [`chicago-fx-list-07`](chicago-fx-list-07/) | KNOWN-DEVIATION | `source-frame-084` | 38741 | battery, footer, glyphs, body |
| [`chicago-fx-list-08`](chicago-fx-list-08/) | KNOWN-DEVIATION | `source-frame-046` | 42831 | battery, footer, glyphs, body |
| [`chicago-fx-list-09`](chicago-fx-list-09/) | KNOWN-DEVIATION | `source-frame-053` | 42355 | battery, footer, glyphs, body |
| [`chicago-fx-list-10`](chicago-fx-list-10/) | KNOWN-DEVIATION | `source-frame-047` | 40437 | battery, footer, glyphs, body |
| [`chicago-fx-list-11`](chicago-fx-list-11/) | KNOWN-DEVIATION | `source-frame-047` | 40437 | battery, footer, glyphs, body |
| [`chicago-mix-00`](chicago-mix-00/) | KNOWN-DEVIATION | `mix-parts-live` | 57600 | battery, footer, glyphs, pan, body |
| [`chicago-mix-00-touch`](chicago-mix-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`chicago-mix-01`](chicago-mix-01/) | KNOWN-DEVIATION | `mix-909-00` | 57600 | battery, footer, glyphs, pan, body |
| [`chicago-mix-01-touch`](chicago-mix-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`chicago-mix-02`](chicago-mix-02/) | KNOWN-DEVIATION | `mix-808-00` | 57600 | battery, footer, glyphs, pan, body |
| [`chicago-mix-02-touch`](chicago-mix-02-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`chicago-mix-03`](chicago-mix-03/) | KNOWN-DEVIATION | `mix-master` | 57600 | battery, footer, glyphs, body |
| [`chicago-mix-03-touch`](chicago-mix-03-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`chicago-mix-04`](chicago-mix-04/) | KNOWN-DEVIATION | `mix-wave` | 57600 | battery, footer, glyphs, body |
| [`chicago-mix-04-touch`](chicago-mix-04-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`chicago-mix-list-00`](chicago-mix-list-00/) | KNOWN-DEVIATION | `source-frame-053` | 36628 | battery, footer, glyphs, body |
| [`chicago-mix-list-01`](chicago-mix-list-01/) | KNOWN-DEVIATION | `source-frame-078` | 36455 | battery, footer, glyphs, body |
| [`chicago-mix-list-02`](chicago-mix-list-02/) | KNOWN-DEVIATION | `source-frame-053` | 39389 | battery, footer, glyphs, body |
| [`chicago-mix-list-03`](chicago-mix-list-03/) | KNOWN-DEVIATION | `source-frame-053` | 39415 | battery, footer, glyphs, body |
| [`chicago-mix-list-04`](chicago-mix-list-04/) | KNOWN-DEVIATION | `source-frame-053` | 38476 | battery, footer, glyphs, body |
| [`chicago-mix-list-05`](chicago-mix-list-05/) | KNOWN-DEVIATION | `source-frame-053` | 38966 | battery, footer, glyphs, body |
| [`chicago-mix-list-06`](chicago-mix-list-06/) | KNOWN-DEVIATION | `source-frame-053` | 39467 | battery, footer, glyphs, body |
| [`chicago-mix-list-07`](chicago-mix-list-07/) | KNOWN-DEVIATION | `source-frame-054` | 40627 | battery, footer, glyphs, body |
| [`chicago-mix-list-08`](chicago-mix-list-08/) | KNOWN-DEVIATION | `source-frame-054` | 40858 | battery, footer, glyphs, body |
| [`chicago-mix-list-09`](chicago-mix-list-09/) | KNOWN-DEVIATION | `source-frame-053` | 39814 | battery, footer, glyphs, body |
| [`chicago-mix-list-10`](chicago-mix-list-10/) | KNOWN-DEVIATION | `source-frame-054` | 40863 | battery, footer, glyphs, body |
| [`chicago-mix-list-11`](chicago-mix-list-11/) | KNOWN-DEVIATION | `source-frame-054` | 40621 | battery, footer, glyphs, body |
| [`chicago-mix-list-12`](chicago-mix-list-12/) | KNOWN-DEVIATION | `source-frame-054` | 40877 | battery, footer, glyphs, body |
| [`chicago-mix-list-13`](chicago-mix-list-13/) | KNOWN-DEVIATION | `source-frame-062` | 39743 | battery, footer, glyphs, body |
| [`chicago-mix-list-14`](chicago-mix-list-14/) | KNOWN-DEVIATION | `source-frame-062` | 39632 | battery, footer, glyphs, body |
| [`chicago-mix-list-15`](chicago-mix-list-15/) | KNOWN-DEVIATION | `source-frame-062` | 39507 | battery, footer, glyphs, body |
| [`chicago-mix-list-16`](chicago-mix-list-16/) | KNOWN-DEVIATION | `source-frame-055` | 38025 | battery, footer, glyphs, body |
| [`chicago-mix-list-17`](chicago-mix-list-17/) | KNOWN-DEVIATION | `source-frame-084` | 38712 | battery, footer, glyphs, body |
| [`chicago-mix-list-18`](chicago-mix-list-18/) | KNOWN-DEVIATION | `source-frame-055` | 44012 | battery, footer, glyphs, body |
| [`chicago-mix-list-19`](chicago-mix-list-19/) | KNOWN-DEVIATION | `source-frame-053` | 42200 | battery, footer, glyphs, body |
| [`chicago-mix-list-20`](chicago-mix-list-20/) | KNOWN-DEVIATION | `source-frame-056` | 40032 | battery, footer, glyphs, body |
| [`chicago-mix-list-21`](chicago-mix-list-21/) | KNOWN-DEVIATION | `source-frame-056` | 40068 | battery, footer, glyphs, body |
| [`chicago-mix-list-22`](chicago-mix-list-22/) | KNOWN-DEVIATION | `source-frame-056` | 39835 | battery, footer, glyphs, body |
| [`chicago-mix-list-23`](chicago-mix-list-23/) | KNOWN-DEVIATION | `source-frame-056` | 40063 | battery, footer, glyphs, body |
| [`chicago-mix-list-24`](chicago-mix-list-24/) | KNOWN-DEVIATION | `source-frame-053` | 39576 | battery, footer, glyphs, body |
| [`chicago-mix-list-25`](chicago-mix-list-25/) | KNOWN-DEVIATION | `source-frame-038` | 39229 | battery, footer, glyphs, body |
| [`chicago-mix-list-26`](chicago-mix-list-26/) | KNOWN-DEVIATION | `source-frame-038` | 39040 | battery, footer, glyphs, body |
| [`chicago-mix-list-27`](chicago-mix-list-27/) | KNOWN-DEVIATION | `source-frame-062` | 39154 | battery, footer, glyphs, body |
| [`chicago-mix-list-28`](chicago-mix-list-28/) | KNOWN-DEVIATION | `source-frame-062` | 39154 | battery, footer, glyphs, body |
| [`chicago-home-00`](chicago-home-00/) | KNOWN-DEVIATION | `home-01-16` | 57600 | battery, footer, glyphs, body |
| [`chicago-home-00-touch`](chicago-home-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`chicago-home-list-00`](chicago-home-list-00/) | KNOWN-DEVIATION | `source-frame-001` | 31372 | battery, footer, glyphs, body |
| [`chicago-home-list-01`](chicago-home-list-01/) | KNOWN-DEVIATION | `source-frame-001` | 33831 | battery, footer, glyphs, body |
| [`chicago-home-list-02`](chicago-home-list-02/) | KNOWN-DEVIATION | `source-frame-001` | 33900 | battery, footer, glyphs, body |
| [`chicago-home-list-03`](chicago-home-list-03/) | KNOWN-DEVIATION | `source-frame-001` | 33857 | battery, footer, glyphs, body |
| [`chicago-gen-00`](chicago-gen-00/) | KNOWN-DEVIATION | `source-frame-040` | 25191 | battery, footer, glyphs, body |
| [`chicago-gen-00-touch`](chicago-gen-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`chicago-gen-01`](chicago-gen-01/) | KNOWN-DEVIATION | `source-frame-041` | 24562 | battery, footer, glyphs, body |
| [`chicago-gen-01-touch`](chicago-gen-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57598 | battery, footer, glyphs, body |
| [`chicago-gen-list-00`](chicago-gen-list-00/) | KNOWN-DEVIATION | `source-frame-042` | 34602 | battery, footer, glyphs, body |
| [`chicago-gen-list-01`](chicago-gen-list-01/) | KNOWN-DEVIATION | `source-frame-078` | 36967 | battery, footer, glyphs, body |
| [`chicago-gen-list-02`](chicago-gen-list-02/) | KNOWN-DEVIATION | `source-frame-038` | 38920 | battery, footer, glyphs, body |
| [`chicago-gen-list-03`](chicago-gen-list-03/) | KNOWN-DEVIATION | `source-frame-038` | 39469 | battery, footer, glyphs, body |
| [`chicago-gen-list-04`](chicago-gen-list-04/) | KNOWN-DEVIATION | `source-frame-038` | 39478 | battery, footer, glyphs, body |
| [`chicago-gen-list-05`](chicago-gen-list-05/) | KNOWN-DEVIATION | `source-frame-038` | 39178 | battery, footer, glyphs, body |
| [`chicago-gen-list-06`](chicago-gen-list-06/) | KNOWN-DEVIATION | `retained-family-06` | 57595 | battery, footer, glyphs, body |
| [`chicago-gen-list-07`](chicago-gen-list-07/) | KNOWN-DEVIATION | `retained-family-06` | 57595 | battery, footer, glyphs, body |
| [`miami-seq-00`](miami-seq-00/) | KNOWN-DEVIATION | `source-frame-071` | 21876 | battery, footer, glyphs, body |
| [`miami-seq-00-touch`](miami-seq-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57598 | battery, footer, glyphs, body |
| [`miami-seq-00-held`](miami-seq-00-held/) | KNOWN-DEVIATION | `selected-held-initial` | 24512 | battery, footer, glyphs, body |
| [`miami-seq-01`](miami-seq-01/) | KNOWN-DEVIATION | `source-frame-072` | 21313 | battery, footer, glyphs, body |
| [`miami-seq-01-touch`](miami-seq-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`miami-seq-01-held`](miami-seq-01-held/) | KNOWN-DEVIATION | `selected-held-initial` | 24512 | battery, footer, glyphs, body |
| [`miami-seq-02`](miami-seq-02/) | KNOWN-DEVIATION | `source-frame-070` | 24501 | battery, footer, glyphs, body |
| [`miami-seq-02-touch`](miami-seq-02-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`miami-seq-02-held`](miami-seq-02-held/) | KNOWN-DEVIATION | `selected-held-initial` | 24512 | battery, footer, glyphs, body |
| [`miami-seq-list-00`](miami-seq-list-00/) | KNOWN-DEVIATION | `source-frame-073` | 32102 | battery, footer, glyphs, body |
| [`miami-seq-list-01`](miami-seq-list-01/) | KNOWN-DEVIATION | `source-frame-073` | 36844 | battery, footer, glyphs, body |
| [`miami-seq-list-02`](miami-seq-list-02/) | KNOWN-DEVIATION | `source-frame-073` | 36390 | battery, footer, glyphs, body |
| [`miami-seq-list-03`](miami-seq-list-03/) | KNOWN-DEVIATION | `source-frame-073` | 36469 | battery, footer, glyphs, body |
| [`miami-seq-list-04`](miami-seq-list-04/) | KNOWN-DEVIATION | `source-frame-073` | 36178 | battery, footer, glyphs, body |
| [`miami-seq-list-05`](miami-seq-list-05/) | KNOWN-DEVIATION | `source-frame-055` | 38590 | battery, footer, glyphs, body |
| [`miami-seq-list-06`](miami-seq-list-06/) | KNOWN-DEVIATION | `source-frame-055` | 38590 | battery, footer, glyphs, body |
| [`miami-seq-list-07`](miami-seq-list-07/) | KNOWN-DEVIATION | `source-frame-055` | 38590 | battery, footer, glyphs, body |
| [`miami-para-00`](miami-para-00/) | KNOWN-DEVIATION | `source-frame-011` | 28559 | battery, footer, glyphs, body |
| [`miami-para-00-touch`](miami-para-00-touch/) | KNOWN-DEVIATION | `touch-master-2-deadline` | 57600 | battery, footer, glyphs, body |
| [`miami-para-01`](miami-para-01/) | KNOWN-DEVIATION | `source-frame-012` | 28592 | battery, footer, glyphs, body |
| [`miami-para-01-touch`](miami-para-01-touch/) | KNOWN-DEVIATION | `touch-master-2-deadline` | 57600 | battery, footer, glyphs, body |
| [`miami-para-02`](miami-para-02/) | KNOWN-DEVIATION | `source-frame-013` | 28648 | battery, footer, glyphs, body |
| [`miami-para-02-touch`](miami-para-02-touch/) | KNOWN-DEVIATION | `touch-master-2-deadline` | 57600 | battery, footer, glyphs, body |
| [`miami-para-03`](miami-para-03/) | KNOWN-DEVIATION | `source-frame-014` | 28755 | battery, footer, glyphs, body |
| [`miami-para-03-touch`](miami-para-03-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`miami-para-04`](miami-para-04/) | KNOWN-DEVIATION | `source-frame-015` | 18953 | battery, footer, glyphs, body |
| [`miami-para-04-touch`](miami-para-04-touch/) | KNOWN-DEVIATION | `touch-master-2-deadline` | 57600 | battery, footer, glyphs, body |
| [`miami-para-05`](miami-para-05/) | KNOWN-DEVIATION | `source-frame-016` | 18710 | battery, footer, glyphs, body |
| [`miami-para-05-touch`](miami-para-05-touch/) | KNOWN-DEVIATION | `touch-master-2-deadline` | 57600 | battery, footer, glyphs, body |
| [`miami-para-list-00`](miami-para-list-00/) | KNOWN-DEVIATION | `source-frame-017` | 26157 | battery, glyphs, body |
| [`miami-para-list-01`](miami-para-list-01/) | KNOWN-DEVIATION | `source-frame-017` | 30616 | battery, glyphs, body |
| [`miami-para-list-02`](miami-para-list-02/) | KNOWN-DEVIATION | `source-frame-017` | 30684 | battery, glyphs, body |
| [`miami-para-list-03`](miami-para-list-03/) | KNOWN-DEVIATION | `source-frame-017` | 30708 | battery, glyphs, body |
| [`miami-para-list-04`](miami-para-list-04/) | KNOWN-DEVIATION | `source-frame-017` | 30628 | battery, glyphs, body |
| [`miami-para-list-05`](miami-para-list-05/) | KNOWN-DEVIATION | `source-frame-017` | 30638 | battery, glyphs, body |
| [`miami-para-list-06`](miami-para-list-06/) | KNOWN-DEVIATION | `source-frame-018` | 31325 | battery, glyphs, body |
| [`miami-para-list-07`](miami-para-list-07/) | KNOWN-DEVIATION | `source-frame-029` | 30831 | battery, glyphs, body |
| [`miami-para-list-08`](miami-para-list-08/) | KNOWN-DEVIATION | `source-frame-018` | 31325 | battery, glyphs, body |
| [`miami-para-list-09`](miami-para-list-09/) | KNOWN-DEVIATION | `source-frame-018` | 31299 | battery, glyphs, body |
| [`miami-para-list-10`](miami-para-list-10/) | KNOWN-DEVIATION | `source-frame-030` | 31491 | battery, glyphs, body |
| [`miami-para-list-11`](miami-para-list-11/) | KNOWN-DEVIATION | `source-frame-018` | 31238 | battery, glyphs, body |
| [`miami-para-list-12`](miami-para-list-12/) | KNOWN-DEVIATION | `source-frame-018` | 31354 | battery, glyphs, body |
| [`miami-para-list-13`](miami-para-list-13/) | KNOWN-DEVIATION | `source-frame-031` | 30876 | battery, glyphs, body |
| [`miami-para-list-14`](miami-para-list-14/) | KNOWN-DEVIATION | `source-frame-018` | 31344 | battery, glyphs, body |
| [`miami-para-list-15`](miami-para-list-15/) | KNOWN-DEVIATION | `source-frame-018` | 31293 | battery, glyphs, body |
| [`miami-para-list-16`](miami-para-list-16/) | KNOWN-DEVIATION | `source-frame-018` | 31326 | battery, glyphs, body |
| [`miami-para-list-17`](miami-para-list-17/) | KNOWN-DEVIATION | `source-frame-019` | 30953 | battery, glyphs, body |
| [`miami-para-list-18`](miami-para-list-18/) | KNOWN-DEVIATION | `source-frame-019` | 31479 | battery, glyphs, body |
| [`miami-fx-00`](miami-fx-00/) | KNOWN-DEVIATION | `fx-reverb-direct` | 57600 | battery, footer, glyphs, body |
| [`miami-fx-00-touch`](miami-fx-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`miami-fx-01`](miami-fx-01/) | KNOWN-DEVIATION | `fx-delay-direct` | 57600 | battery, footer, glyphs, body |
| [`miami-fx-01-touch`](miami-fx-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`miami-fx-list-00`](miami-fx-list-00/) | KNOWN-DEVIATION | `source-frame-046` | 36785 | battery, footer, glyphs, body |
| [`miami-fx-list-01`](miami-fx-list-01/) | KNOWN-DEVIATION | `source-frame-073` | 35355 | battery, footer, glyphs, body |
| [`miami-fx-list-02`](miami-fx-list-02/) | KNOWN-DEVIATION | `source-frame-046` | 39751 | battery, footer, glyphs, body |
| [`miami-fx-list-03`](miami-fx-list-03/) | KNOWN-DEVIATION | `source-frame-046` | 38935 | battery, footer, glyphs, body |
| [`miami-fx-list-04`](miami-fx-list-04/) | KNOWN-DEVIATION | `source-frame-046` | 39362 | battery, footer, glyphs, body |
| [`miami-fx-list-05`](miami-fx-list-05/) | KNOWN-DEVIATION | `source-frame-046` | 39401 | battery, footer, glyphs, body |
| [`miami-fx-list-06`](miami-fx-list-06/) | KNOWN-DEVIATION | `source-frame-055` | 38000 | battery, footer, glyphs, body |
| [`miami-fx-list-07`](miami-fx-list-07/) | KNOWN-DEVIATION | `source-frame-084` | 38741 | battery, footer, glyphs, body |
| [`miami-fx-list-08`](miami-fx-list-08/) | KNOWN-DEVIATION | `source-frame-046` | 42831 | battery, footer, glyphs, body |
| [`miami-fx-list-09`](miami-fx-list-09/) | KNOWN-DEVIATION | `source-frame-053` | 42355 | battery, footer, glyphs, body |
| [`miami-fx-list-10`](miami-fx-list-10/) | KNOWN-DEVIATION | `source-frame-047` | 40437 | battery, footer, glyphs, body |
| [`miami-fx-list-11`](miami-fx-list-11/) | KNOWN-DEVIATION | `source-frame-047` | 40437 | battery, footer, glyphs, body |
| [`miami-mix-00`](miami-mix-00/) | KNOWN-DEVIATION | `mix-parts-live` | 57600 | battery, footer, glyphs, pan, body |
| [`miami-mix-00-touch`](miami-mix-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`miami-mix-01`](miami-mix-01/) | KNOWN-DEVIATION | `mix-909-00` | 57600 | battery, footer, glyphs, pan, body |
| [`miami-mix-01-touch`](miami-mix-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`miami-mix-02`](miami-mix-02/) | KNOWN-DEVIATION | `mix-808-00` | 57600 | battery, footer, glyphs, pan, body |
| [`miami-mix-02-touch`](miami-mix-02-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, pan, body |
| [`miami-mix-03`](miami-mix-03/) | KNOWN-DEVIATION | `mix-master` | 57600 | battery, footer, glyphs, body |
| [`miami-mix-03-touch`](miami-mix-03-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`miami-mix-04`](miami-mix-04/) | KNOWN-DEVIATION | `mix-wave` | 57600 | battery, footer, glyphs, body |
| [`miami-mix-04-touch`](miami-mix-04-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`miami-mix-list-00`](miami-mix-list-00/) | KNOWN-DEVIATION | `source-frame-053` | 36628 | battery, footer, glyphs, body |
| [`miami-mix-list-01`](miami-mix-list-01/) | KNOWN-DEVIATION | `source-frame-053` | 39512 | battery, footer, glyphs, body |
| [`miami-mix-list-02`](miami-mix-list-02/) | KNOWN-DEVIATION | `source-frame-053` | 39389 | battery, footer, glyphs, body |
| [`miami-mix-list-03`](miami-mix-list-03/) | KNOWN-DEVIATION | `source-frame-053` | 39415 | battery, footer, glyphs, body |
| [`miami-mix-list-04`](miami-mix-list-04/) | KNOWN-DEVIATION | `source-frame-053` | 38476 | battery, footer, glyphs, body |
| [`miami-mix-list-05`](miami-mix-list-05/) | KNOWN-DEVIATION | `source-frame-053` | 38966 | battery, footer, glyphs, body |
| [`miami-mix-list-06`](miami-mix-list-06/) | KNOWN-DEVIATION | `source-frame-053` | 39467 | battery, footer, glyphs, body |
| [`miami-mix-list-07`](miami-mix-list-07/) | KNOWN-DEVIATION | `source-frame-054` | 40627 | battery, footer, glyphs, body |
| [`miami-mix-list-08`](miami-mix-list-08/) | KNOWN-DEVIATION | `source-frame-054` | 40858 | battery, footer, glyphs, body |
| [`miami-mix-list-09`](miami-mix-list-09/) | KNOWN-DEVIATION | `source-frame-053` | 39814 | battery, footer, glyphs, body |
| [`miami-mix-list-10`](miami-mix-list-10/) | KNOWN-DEVIATION | `source-frame-054` | 40863 | battery, footer, glyphs, body |
| [`miami-mix-list-11`](miami-mix-list-11/) | KNOWN-DEVIATION | `source-frame-054` | 40621 | battery, footer, glyphs, body |
| [`miami-mix-list-12`](miami-mix-list-12/) | KNOWN-DEVIATION | `source-frame-054` | 40877 | battery, footer, glyphs, body |
| [`miami-mix-list-13`](miami-mix-list-13/) | KNOWN-DEVIATION | `source-frame-062` | 39743 | battery, footer, glyphs, body |
| [`miami-mix-list-14`](miami-mix-list-14/) | KNOWN-DEVIATION | `source-frame-062` | 39632 | battery, footer, glyphs, body |
| [`miami-mix-list-15`](miami-mix-list-15/) | KNOWN-DEVIATION | `source-frame-062` | 39507 | battery, footer, glyphs, body |
| [`miami-mix-list-16`](miami-mix-list-16/) | KNOWN-DEVIATION | `source-frame-055` | 38025 | battery, footer, glyphs, body |
| [`miami-mix-list-17`](miami-mix-list-17/) | KNOWN-DEVIATION | `source-frame-084` | 38712 | battery, footer, glyphs, body |
| [`miami-mix-list-18`](miami-mix-list-18/) | KNOWN-DEVIATION | `source-frame-055` | 44012 | battery, footer, glyphs, body |
| [`miami-mix-list-19`](miami-mix-list-19/) | KNOWN-DEVIATION | `source-frame-053` | 42200 | battery, footer, glyphs, body |
| [`miami-mix-list-20`](miami-mix-list-20/) | KNOWN-DEVIATION | `source-frame-056` | 40032 | battery, footer, glyphs, body |
| [`miami-mix-list-21`](miami-mix-list-21/) | KNOWN-DEVIATION | `source-frame-056` | 40068 | battery, footer, glyphs, body |
| [`miami-mix-list-22`](miami-mix-list-22/) | KNOWN-DEVIATION | `source-frame-056` | 39835 | battery, footer, glyphs, body |
| [`miami-mix-list-23`](miami-mix-list-23/) | KNOWN-DEVIATION | `source-frame-056` | 40063 | battery, footer, glyphs, body |
| [`miami-mix-list-24`](miami-mix-list-24/) | KNOWN-DEVIATION | `source-frame-053` | 39576 | battery, footer, glyphs, body |
| [`miami-mix-list-25`](miami-mix-list-25/) | KNOWN-DEVIATION | `source-frame-038` | 39229 | battery, footer, glyphs, body |
| [`miami-mix-list-26`](miami-mix-list-26/) | KNOWN-DEVIATION | `source-frame-038` | 39040 | battery, footer, glyphs, body |
| [`miami-mix-list-27`](miami-mix-list-27/) | KNOWN-DEVIATION | `source-frame-062` | 39154 | battery, footer, glyphs, body |
| [`miami-mix-list-28`](miami-mix-list-28/) | KNOWN-DEVIATION | `source-frame-062` | 39154 | battery, footer, glyphs, body |
| [`miami-home-00`](miami-home-00/) | KNOWN-DEVIATION | `home-01-16` | 57600 | battery, footer, glyphs, body |
| [`miami-home-00-touch`](miami-home-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57600 | battery, footer, glyphs, body |
| [`miami-home-list-00`](miami-home-list-00/) | KNOWN-DEVIATION | `source-frame-001` | 31372 | battery, footer, glyphs, body |
| [`miami-home-list-01`](miami-home-list-01/) | KNOWN-DEVIATION | `source-frame-001` | 33831 | battery, footer, glyphs, body |
| [`miami-home-list-02`](miami-home-list-02/) | KNOWN-DEVIATION | `source-frame-001` | 33900 | battery, footer, glyphs, body |
| [`miami-home-list-03`](miami-home-list-03/) | KNOWN-DEVIATION | `source-frame-001` | 33857 | battery, footer, glyphs, body |
| [`miami-gen-00`](miami-gen-00/) | KNOWN-DEVIATION | `source-frame-036` | 25197 | battery, footer, glyphs, body |
| [`miami-gen-00-touch`](miami-gen-00-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57599 | battery, footer, glyphs, body |
| [`miami-gen-01`](miami-gen-01/) | KNOWN-DEVIATION | `source-frame-037` | 24547 | battery, footer, glyphs, body |
| [`miami-gen-01-touch`](miami-gen-01-touch/) | KNOWN-DEVIATION | `touch-master-0-queued` | 57598 | battery, footer, glyphs, body |
| [`miami-gen-list-00`](miami-gen-list-00/) | KNOWN-DEVIATION | `source-frame-038` | 34399 | battery, footer, glyphs, body |
| [`miami-gen-list-01`](miami-gen-list-01/) | KNOWN-DEVIATION | `source-frame-073` | 36785 | battery, footer, glyphs, body |
| [`miami-gen-list-02`](miami-gen-list-02/) | KNOWN-DEVIATION | `source-frame-038` | 38920 | battery, footer, glyphs, body |
| [`miami-gen-list-03`](miami-gen-list-03/) | KNOWN-DEVIATION | `source-frame-038` | 39469 | battery, footer, glyphs, body |
| [`miami-gen-list-04`](miami-gen-list-04/) | KNOWN-DEVIATION | `source-frame-038` | 39478 | battery, footer, glyphs, body |
| [`miami-gen-list-05`](miami-gen-list-05/) | KNOWN-DEVIATION | `source-frame-038` | 39178 | battery, footer, glyphs, body |
| [`miami-gen-list-06`](miami-gen-list-06/) | KNOWN-DEVIATION | `retained-family-06` | 57595 | battery, footer, glyphs, body |
| [`miami-gen-list-07`](miami-gen-list-07/) | KNOWN-DEVIATION | `retained-family-06` | 57595 | battery, footer, glyphs, body |
| [`global-index`](global-index/) | KNOWN-DEVIATION | `source-frame-086` | 31140 | battery, footer, glyphs, body |
| [`global-0-row-00`](global-0-row-00/) | KNOWN-DEVIATION | `source-frame-087` | 36270 | battery, footer, glyphs, body |
| [`global-0-row-01`](global-0-row-01/) | KNOWN-DEVIATION | `source-frame-087` | 40026 | battery, footer, glyphs, body |
| [`global-0-row-02`](global-0-row-02/) | KNOWN-DEVIATION | `source-frame-087` | 39971 | battery, footer, glyphs, body |
| [`global-0-row-03`](global-0-row-03/) | KNOWN-DEVIATION | `source-frame-087` | 40028 | battery, footer, glyphs, body |
| [`global-0-row-04`](global-0-row-04/) | KNOWN-DEVIATION | `source-frame-087` | 40045 | battery, footer, glyphs, body |
| [`global-0-row-05`](global-0-row-05/) | KNOWN-DEVIATION | `source-frame-087` | 39883 | battery, footer, glyphs, body |
| [`global-0-row-06`](global-0-row-06/) | KNOWN-DEVIATION | `source-frame-087` | 40100 | battery, footer, glyphs, body |
| [`global-0-row-07`](global-0-row-07/) | KNOWN-DEVIATION | `source-frame-087` | 40290 | battery, footer, glyphs, body |
| [`global-1-row-00`](global-1-row-00/) | KNOWN-DEVIATION | `source-frame-089` | 35928 | battery, footer, glyphs, body |
| [`global-1-row-01`](global-1-row-01/) | KNOWN-DEVIATION | `source-frame-089` | 39043 | battery, footer, glyphs, body |
| [`global-1-row-02`](global-1-row-02/) | KNOWN-DEVIATION | `source-frame-089` | 39942 | battery, footer, glyphs, body |
| [`global-1-row-03`](global-1-row-03/) | KNOWN-DEVIATION | `source-frame-089` | 40401 | battery, footer, glyphs, body |
| [`global-1-row-04`](global-1-row-04/) | KNOWN-DEVIATION | `source-frame-089` | 40399 | battery, footer, glyphs, body |
| [`global-1-row-05`](global-1-row-05/) | KNOWN-DEVIATION | `source-frame-089` | 40178 | battery, footer, glyphs, body |
| [`global-1-row-06`](global-1-row-06/) | KNOWN-DEVIATION | `source-frame-089` | 40472 | battery, footer, glyphs, body |
| [`global-1-row-07`](global-1-row-07/) | KNOWN-DEVIATION | `source-frame-000` | 46824 | battery, footer, glyphs, body |
| [`global-1-row-08`](global-1-row-08/) | KNOWN-DEVIATION | `source-frame-000` | 46821 | battery, footer, glyphs, body |
| [`global-1-row-09`](global-1-row-09/) | KNOWN-DEVIATION | `source-frame-000` | 46852 | battery, footer, glyphs, body |
| [`global-2-row-00`](global-2-row-00/) | KNOWN-DEVIATION | `source-frame-091` | 27089 | battery, footer, glyphs, body |
| [`global-2-row-01`](global-2-row-01/) | KNOWN-DEVIATION | `source-frame-091` | 31027 | battery, footer, glyphs, body |
| [`global-2-row-02`](global-2-row-02/) | KNOWN-DEVIATION | `source-frame-091` | 31549 | battery, footer, glyphs, body |
| [`global-3-row-00`](global-3-row-00/) | KNOWN-DEVIATION | `source-frame-092` | 33354 | battery, footer, glyphs, body |
| [`global-3-row-01`](global-3-row-01/) | KNOWN-DEVIATION | `source-frame-092` | 37208 | battery, footer, glyphs, body |
| [`global-3-row-02`](global-3-row-02/) | KNOWN-DEVIATION | `source-frame-092` | 37346 | battery, footer, glyphs, body |
| [`global-3-row-03`](global-3-row-03/) | KNOWN-DEVIATION | `source-frame-092` | 36944 | battery, footer, glyphs, body |
| [`global-3-row-04`](global-3-row-04/) | KNOWN-DEVIATION | `source-frame-092` | 36946 | battery, footer, glyphs, body |
| [`global-4-row-00`](global-4-row-00/) | KNOWN-DEVIATION | `source-frame-093` | 27241 | battery, footer, glyphs, body |
| [`global-4-row-01`](global-4-row-01/) | KNOWN-DEVIATION | `source-frame-093` | 31048 | battery, footer, glyphs, body |
| [`global-4-row-02`](global-4-row-02/) | KNOWN-DEVIATION | `source-frame-093` | 31187 | battery, footer, glyphs, body |
| [`theme-green-p0-seq`](theme-green-p0-seq/) | KNOWN-DEVIATION | `source-frame-066` | 40670 | battery, footer, glyphs, body |
| [`theme-green-p0-para`](theme-green-p0-para/) | KNOWN-DEVIATION | `source-frame-002` | 19089 | battery, footer, glyphs, drumpara, body |
| [`theme-green-p1-para`](theme-green-p1-para/) | KNOWN-DEVIATION | `source-frame-007` | 17037 | battery, footer, glyphs, drumpara, body |
| [`theme-green-p3-para`](theme-green-p3-para/) | KNOWN-DEVIATION | `source-frame-021` | 27010 | battery, footer, glyphs, body |
| [`theme-green-p4-para`](theme-green-p4-para/) | KNOWN-DEVIATION | `source-frame-011` | 28559 | battery, footer, glyphs, body |
| [`theme-green-p4-gen`](theme-green-p4-gen/) | KNOWN-DEVIATION | `source-frame-036` | 25202 | battery, footer, glyphs, body |
| [`theme-green-p2-seq`](theme-green-p2-seq/) | KNOWN-DEVIATION | `retained-family-10` | 53569 | battery, footer, glyphs, body |
| [`theme-green-p0-fx`](theme-green-p0-fx/) | KNOWN-DEVIATION | `source-frame-044` | 35494 | battery, footer, glyphs, body |
| [`theme-green-p0-mix`](theme-green-p0-mix/) | KNOWN-DEVIATION | `source-frame-048` | 31049 | battery, footer, glyphs, body |
| [`theme-green-p0-home`](theme-green-p0-home/) | KNOWN-DEVIATION | `source-frame-000` | 40410 | battery, footer, glyphs, body |
| [`theme-green-global`](theme-green-global/) | KNOWN-DEVIATION | `source-frame-086` | 31145 | battery, footer, glyphs, body |
| [`theme-cyan-p0-seq`](theme-cyan-p0-seq/) | KNOWN-DEVIATION | `source-frame-066` | 40670 | battery, footer, glyphs, body |
| [`theme-cyan-p0-para`](theme-cyan-p0-para/) | KNOWN-DEVIATION | `source-frame-002` | 19089 | battery, footer, glyphs, drumpara, body |
| [`theme-cyan-p1-para`](theme-cyan-p1-para/) | KNOWN-DEVIATION | `source-frame-007` | 17037 | battery, footer, glyphs, drumpara, body |
| [`theme-cyan-p3-para`](theme-cyan-p3-para/) | KNOWN-DEVIATION | `source-frame-021` | 27010 | battery, footer, glyphs, body |
| [`theme-cyan-p4-para`](theme-cyan-p4-para/) | KNOWN-DEVIATION | `source-frame-011` | 28559 | battery, footer, glyphs, body |
| [`theme-cyan-p4-gen`](theme-cyan-p4-gen/) | KNOWN-DEVIATION | `source-frame-036` | 25202 | battery, footer, glyphs, body |
| [`theme-cyan-p2-seq`](theme-cyan-p2-seq/) | KNOWN-DEVIATION | `retained-family-10` | 53569 | battery, footer, glyphs, body |
| [`theme-cyan-p0-fx`](theme-cyan-p0-fx/) | KNOWN-DEVIATION | `source-frame-044` | 35494 | battery, footer, glyphs, body |
| [`theme-cyan-p0-mix`](theme-cyan-p0-mix/) | KNOWN-DEVIATION | `source-frame-048` | 31049 | battery, footer, glyphs, body |
| [`theme-cyan-p0-home`](theme-cyan-p0-home/) | KNOWN-DEVIATION | `source-frame-000` | 40410 | battery, footer, glyphs, body |
| [`theme-cyan-global`](theme-cyan-global/) | KNOWN-DEVIATION | `source-frame-086` | 31145 | battery, footer, glyphs, body |
| [`theme-red-p0-seq`](theme-red-p0-seq/) | KNOWN-DEVIATION | `source-frame-066` | 40670 | battery, footer, glyphs, body |
| [`theme-red-p0-para`](theme-red-p0-para/) | KNOWN-DEVIATION | `source-frame-002` | 19089 | battery, footer, glyphs, drumpara, body |
| [`theme-red-p1-para`](theme-red-p1-para/) | KNOWN-DEVIATION | `source-frame-007` | 17037 | battery, footer, glyphs, drumpara, body |
| [`theme-red-p3-para`](theme-red-p3-para/) | KNOWN-DEVIATION | `source-frame-021` | 27010 | battery, footer, glyphs, body |
| [`theme-red-p4-para`](theme-red-p4-para/) | KNOWN-DEVIATION | `source-frame-011` | 28559 | battery, footer, glyphs, body |
| [`theme-red-p4-gen`](theme-red-p4-gen/) | KNOWN-DEVIATION | `source-frame-036` | 25202 | battery, footer, glyphs, body |
| [`theme-red-p2-seq`](theme-red-p2-seq/) | KNOWN-DEVIATION | `retained-family-10` | 53569 | battery, footer, glyphs, body |
| [`theme-red-p0-fx`](theme-red-p0-fx/) | KNOWN-DEVIATION | `source-frame-044` | 35494 | battery, footer, glyphs, body |
| [`theme-red-p0-mix`](theme-red-p0-mix/) | KNOWN-DEVIATION | `source-frame-048` | 31049 | battery, footer, glyphs, body |
| [`theme-red-p0-home`](theme-red-p0-home/) | KNOWN-DEVIATION | `source-frame-000` | 40410 | battery, footer, glyphs, body |
| [`theme-red-global`](theme-red-global/) | KNOWN-DEVIATION | `source-frame-086` | 31145 | battery, footer, glyphs, body |
| [`theme-mono-p0-seq`](theme-mono-p0-seq/) | KNOWN-DEVIATION | `source-frame-066` | 40670 | battery, footer, glyphs, body |
| [`theme-mono-p0-para`](theme-mono-p0-para/) | KNOWN-DEVIATION | `source-frame-002` | 19089 | battery, footer, glyphs, drumpara, body |
| [`theme-mono-p1-para`](theme-mono-p1-para/) | KNOWN-DEVIATION | `source-frame-007` | 17037 | battery, footer, glyphs, drumpara, body |
| [`theme-mono-p3-para`](theme-mono-p3-para/) | KNOWN-DEVIATION | `source-frame-021` | 27010 | battery, footer, glyphs, body |
| [`theme-mono-p4-para`](theme-mono-p4-para/) | KNOWN-DEVIATION | `source-frame-011` | 28559 | battery, footer, glyphs, body |
| [`theme-mono-p4-gen`](theme-mono-p4-gen/) | KNOWN-DEVIATION | `source-frame-036` | 25202 | battery, footer, glyphs, body |
| [`theme-mono-p2-seq`](theme-mono-p2-seq/) | KNOWN-DEVIATION | `retained-family-10` | 53569 | battery, footer, glyphs, body |
| [`theme-mono-p0-fx`](theme-mono-p0-fx/) | KNOWN-DEVIATION | `source-frame-044` | 35494 | battery, footer, glyphs, body |
| [`theme-mono-p0-mix`](theme-mono-p0-mix/) | KNOWN-DEVIATION | `source-frame-048` | 31049 | battery, footer, glyphs, body |
| [`theme-mono-p0-home`](theme-mono-p0-home/) | KNOWN-DEVIATION | `source-frame-000` | 40410 | battery, footer, glyphs, body |
| [`theme-mono-global`](theme-mono-global/) | KNOWN-DEVIATION | `source-frame-086` | 31145 | battery, footer, glyphs, body |
| [`theme-light-p0-seq`](theme-light-p0-seq/) | KNOWN-DEVIATION | `retained-family-10` | 57600 | battery, footer, glyphs, body |
| [`theme-light-p0-para`](theme-light-p0-para/) | KNOWN-DEVIATION | `drums-909-snare` | 57600 | battery, footer, glyphs, drumpara, body |
| [`theme-light-p1-para`](theme-light-p1-para/) | KNOWN-DEVIATION | `drums-909-snare` | 57600 | battery, footer, glyphs, drumpara, body |
| [`theme-light-p3-para`](theme-light-p3-para/) | KNOWN-DEVIATION | `drums-909-snare` | 57600 | battery, footer, glyphs, body |
| [`theme-light-p4-para`](theme-light-p4-para/) | KNOWN-DEVIATION | `drums-909-snare` | 57600 | battery, footer, glyphs, body |
| [`theme-light-p4-gen`](theme-light-p4-gen/) | KNOWN-DEVIATION | `retained-family-10` | 57600 | battery, footer, glyphs, body |
| [`theme-light-p2-seq`](theme-light-p2-seq/) | KNOWN-DEVIATION | `drums-family-00` | 57600 | battery, footer, glyphs, body |
| [`theme-light-p0-fx`](theme-light-p0-fx/) | KNOWN-DEVIATION | `retained-family-10` | 57600 | battery, footer, glyphs, body |
| [`theme-light-p0-mix`](theme-light-p0-mix/) | KNOWN-DEVIATION | `retained-family-10` | 57600 | battery, footer, glyphs, body |
| [`theme-light-p0-home`](theme-light-p0-home/) | KNOWN-DEVIATION | `retained-family-10` | 57600 | battery, footer, glyphs, body |
| [`theme-light-global`](theme-light-global/) | KNOWN-DEVIATION | `drums-909-snare` | 57600 | battery, footer, glyphs, body |
| [`transport-playing`](transport-playing/) | KNOWN-DEVIATION | `source-frame-066` | 40870 | battery, footer, glyphs, body |
| [`transport-rec`](transport-rec/) | KNOWN-DEVIATION | `source-frame-066` | 40838 | battery, footer, glyphs, body |
| [`help-FX`](help-FX/) | KNOWN-DEVIATION | `source-frame-067` | 27788 | battery, footer, glyphs, body |
| [`help-SEL`](help-SEL/) | KNOWN-DEVIATION | `source-frame-067` | 27838 | battery, footer, glyphs, body |
| [`help-ENV`](help-ENV/) | KNOWN-DEVIATION | `source-frame-067` | 27298 | battery, footer, glyphs, body |
| [`help-LFO`](help-LFO/) | KNOWN-DEVIATION | `source-frame-067` | 27171 | battery, footer, glyphs, body |
| [`help-EDIT`](help-EDIT/) | KNOWN-DEVIATION | `source-frame-067` | 28658 | battery, footer, glyphs, body |
| [`help-GLO`](help-GLO/) | KNOWN-DEVIATION | `source-frame-067` | 27834 | battery, footer, glyphs, body |
| [`help-HOME`](help-HOME/) | KNOWN-DEVIATION | `source-frame-067` | 30334 | battery, footer, glyphs, body |
| [`help-SAVE`](help-SAVE/) | KNOWN-DEVIATION | `source-frame-067` | 29780 | battery, footer, glyphs, body |
| [`help-ARP`](help-ARP/) | KNOWN-DEVIATION | `source-frame-067` | 28118 | battery, footer, glyphs, body |
| [`help-SEQ`](help-SEQ/) | KNOWN-DEVIATION | `source-frame-067` | 27842 | battery, footer, glyphs, body |
| [`help-PLAY`](help-PLAY/) | KNOWN-DEVIATION | `source-frame-067` | 27041 | battery, footer, glyphs, body |
| [`help-REC`](help-REC/) | KNOWN-DEVIATION | `source-frame-067` | 28525 | battery, footer, glyphs, body |
| [`help-OCTdown`](help-OCTdown/) | KNOWN-DEVIATION | `source-frame-067` | 27310 | battery, footer, glyphs, body |
| [`help-OCTup`](help-OCTup/) | KNOWN-DEVIATION | `source-frame-067` | 27310 | battery, footer, glyphs, body |
| [`song-0`](song-0/) | KNOWN-DEVIATION | `source-frame-060` | 21081 | battery, footer, glyphs, body |
| [`song-1`](song-1/) | KNOWN-DEVIATION | `source-frame-060` | 21081 | battery, footer, glyphs, body |
| [`journey-tour-t01_909`](journey-tour-t01_909/) | KNOWN-DEVIATION | `source-frame-007` | 17360 | battery, footer, glyphs, body |
| [`journey-tour-t02_909_readout`](journey-tour-t02_909_readout/) | KNOWN-DEVIATION | `source-frame-007` | 17348 | battery, footer, glyphs, body |
| [`journey-tour-t03_808`](journey-tour-t03_808/) | KNOWN-DEVIATION | `source-frame-007` | 16619 | battery, footer, glyphs, body |
| [`journey-tour-t04_303a`](journey-tour-t04_303a/) | KNOWN-DEVIATION | `source-frame-011` | 28589 | battery, footer, glyphs, body |
| [`journey-tour-t05_303a_held`](journey-tour-t05_303a_held/) | KNOWN-DEVIATION | `source-frame-011` | 28595 | battery, footer, glyphs, body |
| [`journey-tour-t06_tb3po`](journey-tour-t06_tb3po/) | KNOWN-DEVIATION | `source-frame-036` | 27130 | battery, footer, glyphs, body |
| [`journey-tour-t07_new_line_msg`](journey-tour-t07_new_line_msg/) | KNOWN-DEVIATION | `source-frame-036` | 27360 | battery, footer, glyphs, body |
| [`journey-tour-t08_break`](journey-tour-t08_break/) | KNOWN-DEVIATION | `retained-family-10` | 54541 | battery, footer, glyphs, body |
| [`journey-tour-t09_break_loops`](journey-tour-t09_break_loops/) | KNOWN-DEVIATION | `retained-family-10` | 54663 | battery, footer, glyphs, body |
| [`journey-tour-t10_home`](journey-tour-t10_home/) | KNOWN-DEVIATION | `source-frame-000` | 41101 | battery, footer, glyphs, body |
| [`journey-tour-t11_home_mute`](journey-tour-t11_home_mute/) | KNOWN-DEVIATION | `source-frame-000` | 41102 | battery, footer, glyphs, body |
| [`journey-tour-t12_fx`](journey-tour-t12_fx/) | KNOWN-DEVIATION | `source-frame-044` | 35490 | battery, footer, glyphs, body |
| [`journey-tour-t13_mix_comp`](journey-tour-t13_mix_comp/) | KNOWN-DEVIATION | `source-frame-052` | 36565 | battery, footer, glyphs, body |
| [`journey-tour-t14_mix_pump`](journey-tour-t14_mix_pump/) | KNOWN-DEVIATION | `source-frame-052` | 28331 | battery, footer, glyphs, body |
| [`journey-tour-t15_list`](journey-tour-t15_list/) | KNOWN-DEVIATION | `source-frame-053` | 36538 | battery, footer, glyphs, body |
| [`journey-tour-t16_list_edit`](journey-tour-t16_list_edit/) | KNOWN-DEVIATION | `source-frame-053` | 39325 | battery, footer, glyphs, body |
| [`journey-tour-t17_global`](journey-tour-t17_global/) | KNOWN-DEVIATION | `source-frame-086` | 32921 | battery, footer, glyphs, body |
| [`journey-tour-t18_global_reset`](journey-tour-t18_global_reset/) | KNOWN-DEVIATION | `source-frame-092` | 36993 | battery, footer, glyphs, body |
| [`journey-tour-t19_ask`](journey-tour-t19_ask/) | KNOWN-DEVIATION | `source-frame-072` | 26806 | battery, footer, glyphs, body |
| [`journey-tour-t20_cancelled`](journey-tour-t20_cancelled/) | KNOWN-DEVIATION | `source-frame-092` | 36993 | battery, footer, glyphs, body |
| [`journey-tour-t21_ask_clear`](journey-tour-t21_ask_clear/) | KNOWN-DEVIATION | `source-frame-077` | 25761 | battery, footer, glyphs, body |
| [`journey-tour-t22_tempo`](journey-tour-t22_tempo/) | KNOWN-DEVIATION | `source-frame-049` | 43469 | battery, footer, glyphs, body |
| [`journey-song-s01_303a_on_p2`](journey-song-s01_303a_on_p2/) | KNOWN-DEVIATION | `source-frame-070` | 24501 | battery, footer, glyphs, body |
| [`journey-song-s02_home_parts`](journey-song-s02_home_parts/) | KNOWN-DEVIATION | `source-frame-000` | 40483 | battery, footer, glyphs, body |
| [`journey-song-s03_song`](journey-song-s03_song/) | KNOWN-DEVIATION | `source-frame-061` | 31688 | battery, footer, glyphs, body |
| [`journey-song-s04_song_playing`](journey-song-s04_song_playing/) | KNOWN-DEVIATION | `source-frame-061` | 28478 | battery, footer, glyphs, body |
| [`journey-song-s05_motion`](journey-song-s05_motion/) | KNOWN-DEVIATION | `source-frame-013` | 28692 | battery, footer, glyphs, body |
| [`journey-help-h01_save`](journey-help-h01_save/) | KNOWN-DEVIATION | `source-frame-067` | 29780 | battery, footer, glyphs, body |
| [`journey-help-h02_home`](journey-help-h02_home/) | KNOWN-DEVIATION | `source-frame-067` | 30334 | battery, footer, glyphs, body |
| [`journey-help-h03_oct_kbd`](journey-help-h03_oct_kbd/) | KNOWN-DEVIATION | `source-frame-065` | 26731 | battery, footer, glyphs, body |
| [`journey-sound_matrix_chicago-chicago_sound`](journey-sound_matrix_chicago-chicago_sound/) | KNOWN-DEVIATION | `source-frame-021` | 27010 | battery, footer, glyphs, body |
| [`journey-sound_matrix_chicago-chicago_saw`](journey-sound_matrix_chicago-chicago_saw/) | KNOWN-DEVIATION | `source-frame-021` | 27010 | battery, footer, glyphs, body |
| [`journey-sound_matrix_chicago-chicago_mix`](journey-sound_matrix_chicago-chicago_mix/) | KNOWN-DEVIATION | `source-frame-021` | 27019 | battery, footer, glyphs, body |
| [`journey-sound_matrix_chicago-chicago_square`](journey-sound_matrix_chicago-chicago_square/) | KNOWN-DEVIATION | `source-frame-021` | 27044 | battery, footer, glyphs, body |
| [`journey-sound_matrix_chicago-chicago_adsr_idle`](journey-sound_matrix_chicago-chicago_adsr_idle/) | KNOWN-DEVIATION | `source-frame-024` | 26604 | battery, footer, glyphs, body |
| [`journey-sound_matrix_chicago-chicago_adsr_active`](journey-sound_matrix_chicago-chicago_adsr_active/) | KNOWN-DEVIATION | `source-frame-024` | 26865 | battery, footer, glyphs, body |
| [`journey-sound_matrix_chicago-chicago_adsr_done`](journey-sound_matrix_chicago-chicago_adsr_done/) | KNOWN-DEVIATION | `source-frame-024` | 26620 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_sound`](journey-sound_matrix_miami-miami_sound/) | KNOWN-DEVIATION | `source-frame-011` | 28559 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_triangle`](journey-sound_matrix_miami-miami_triangle/) | KNOWN-DEVIATION | `source-frame-011` | 28572 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_square`](journey-sound_matrix_miami-miami_square/) | KNOWN-DEVIATION | `source-frame-011` | 28570 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_xmod_triangle`](journey-sound_matrix_miami-miami_xmod_triangle/) | KNOWN-DEVIATION | `source-frame-012` | 28619 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_xmod_pulse`](journey-sound_matrix_miami-miami_xmod_pulse/) | KNOWN-DEVIATION | `source-frame-012` | 28645 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_filter`](journey-sound_matrix_miami-miami_filter/) | KNOWN-DEVIATION | `source-frame-013` | 28701 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_wobble_triangle`](journey-sound_matrix_miami-miami_wobble_triangle/) | KNOWN-DEVIATION | `source-frame-014` | 28822 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_wobble_square`](journey-sound_matrix_miami-miami_wobble_square/) | KNOWN-DEVIATION | `source-frame-014` | 28851 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_wobble_idle`](journey-sound_matrix_miami-miami_wobble_idle/) | KNOWN-DEVIATION | `source-frame-014` | 28851 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_wobble_active`](journey-sound_matrix_miami-miami_wobble_active/) | KNOWN-DEVIATION | `source-frame-014` | 28828 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_wobble_grace`](journey-sound_matrix_miami-miami_wobble_grace/) | KNOWN-DEVIATION | `source-frame-014` | 28828 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_wobble_sustain`](journey-sound_matrix_miami-miami_wobble_sustain/) | KNOWN-DEVIATION | `source-frame-014` | 28837 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_wobble_done`](journey-sound_matrix_miami-miami_wobble_done/) | KNOWN-DEVIATION | `source-frame-014` | 28837 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_page2`](journey-sound_matrix_miami-miami_page2/) | KNOWN-DEVIATION | `source-frame-015` | 18953 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_presets`](journey-sound_matrix_miami-miami_presets/) | KNOWN-DEVIATION | `source-frame-016` | 18710 | battery, footer, glyphs, body |
| [`journey-sound_matrix_miami-miami_wrap`](journey-sound_matrix_miami-miami_wrap/) | KNOWN-DEVIATION | `source-frame-011` | 28641 | battery, footer, glyphs, body |
| [`journey-sound_page_split-split_909_sd_sound`](journey-sound_page_split-split_909_sd_sound/) | KNOWN-DEVIATION | `source-frame-007` | 17354 | battery, footer, glyphs, body |
| [`journey-sound_page_split-split_909_sd_sound_after_seq`](journey-sound_page_split-split_909_sd_sound_after_seq/) | KNOWN-DEVIATION | `source-frame-007` | 17365 | battery, footer, glyphs, body |
| [`journey-sound_page_split-split_303_roll`](journey-sound_page_split-split_303_roll/) | KNOWN-DEVIATION | `source-frame-070` | 24501 | battery, footer, glyphs, body |
| [`journey-sound_page_split-split_303_sound`](journey-sound_page_split-split_303_sound/) | KNOWN-DEVIATION | `selected-motion-recorded` | 28546 | battery, footer, glyphs, body |
| [`journey-savefeedback-savefailed`](journey-savefeedback-savefailed/) | KNOWN-DEVIATION | `source-frame-086` | 31140 | battery, footer, glyphs, body |
| [`journey-savefeedback-savequeued`](journey-savefeedback-savequeued/) | KNOWN-DEVIATION | `source-frame-066` | 40870 | battery, footer, glyphs, body |
| [`journey-groove-01_boot_909`](journey-groove-01_boot_909/) | KNOWN-DEVIATION | `source-frame-066` | 40662 | battery, footer, glyphs, body |
| [`journey-groove-02_909_grid`](journey-groove-02_909_grid/) | KNOWN-DEVIATION | `source-frame-066` | 40922 | battery, footer, glyphs, body |
| [`journey-groove-03_808_grid`](journey-groove-03_808_grid/) | KNOWN-DEVIATION | `source-frame-068` | 40804 | battery, footer, glyphs, body |
| [`journey-groove-04_tb3po_a`](journey-groove-04_tb3po_a/) | KNOWN-DEVIATION | `source-frame-036` | 26163 | battery, footer, glyphs, body |
| [`journey-groove-05_303b_sound`](journey-groove-05_303b_sound/) | KNOWN-DEVIATION | `source-frame-021` | 27015 | battery, footer, glyphs, body |
| [`journey-groove-06_break`](journey-groove-06_break/) | KNOWN-DEVIATION | `retained-family-10` | 54880 | battery, footer, glyphs, body |
| [`journey-groove-07_playing_break`](journey-groove-07_playing_break/) | KNOWN-DEVIATION | `retained-family-10` | 55184 | battery, footer, glyphs, body |
| [`journey-groove-08_home`](journey-groove-08_home/) | KNOWN-DEVIATION | `source-frame-000` | 41154 | battery, footer, glyphs, body |
| [`journey-groove-09_fx`](journey-groove-09_fx/) | KNOWN-DEVIATION | `source-frame-044` | 35490 | battery, footer, glyphs, body |
| [`journey-groove-10_mix`](journey-groove-10_mix/) | KNOWN-DEVIATION | `source-frame-048` | 31420 | battery, footer, glyphs, body |
| [`journey-patterns-11_chain`](journey-patterns-11_chain/) | KNOWN-DEVIATION | `source-frame-000` | 40438 | battery, footer, glyphs, body |
| [`journey-clocksrc-01_glo_clock_source`](journey-clocksrc-01_glo_clock_source/) | KNOWN-DEVIATION | `source-frame-087` | 36270 | battery, footer, glyphs, body |
| [`journey-clocksrc-02_glo_external`](journey-clocksrc-02_glo_external/) | KNOWN-DEVIATION | `source-frame-087` | 36463 | battery, footer, glyphs, body |
| [`journey-clocksrc-03_waiting_for_clock`](journey-clocksrc-03_waiting_for_clock/) | KNOWN-DEVIATION | `source-frame-066` | 40886 | battery, footer, glyphs, body |
| [`journey-clocksrc-04_following`](journey-clocksrc-04_following/) | KNOWN-DEVIATION | `source-frame-066` | 40902 | battery, footer, glyphs, body |
| [`journey-clocksrc-05_internal`](journey-clocksrc-05_internal/) | KNOWN-DEVIATION | `source-frame-066` | 40902 | battery, footer, glyphs, body |
| [`journey-perf-p01_live`](journey-perf-p01_live/) | KNOWN-DEVIATION | `source-frame-064` | 26929 | battery, footer, glyphs, body |
| [`journey-perf-p02_running`](journey-perf-p02_running/) | KNOWN-DEVIATION | `source-frame-072` | 24550 | battery, footer, glyphs, body |
| [`journey-perf-p03_results`](journey-perf-p03_results/) | KNOWN-DEVIATION | `source-frame-072` | 25838 | battery, footer, glyphs, body |
| [`journey-guard-g01_guard`](journey-guard-g01_guard/) | KNOWN-DEVIATION | `source-frame-036` | 27410 | battery, footer, glyphs, body |
| [`journey-guard-g02_perftest`](journey-guard-g02_perftest/) | KNOWN-DEVIATION | `source-frame-072` | 24603 | battery, footer, glyphs, body |
| [`journey-guard-g03_perf_after`](journey-guard-g03_perf_after/) | KNOWN-DEVIATION | `source-frame-064` | 27045 | battery, footer, glyphs, body |
| [`journey-jam-20_break_amen`](journey-jam-20_break_amen/) | KNOWN-DEVIATION | `retained-family-10` | 54888 | battery, footer, glyphs, body |
| [`journey-jam-21_sends_303`](journey-jam-21_sends_303/) | KNOWN-DEVIATION | `source-frame-048` | 41090 | battery, footer, glyphs, body |
| [`journey-jam-22_sends_break`](journey-jam-22_sends_break/) | KNOWN-DEVIATION | `source-frame-048` | 41107 | battery, footer, glyphs, body |
| [`journey-jam-22b_tape`](journey-jam-22b_tape/) | KNOWN-DEVIATION | `source-frame-045` | 43582 | battery, footer, glyphs, body |
| [`journey-jam-23_home_jam`](journey-jam-23_home_jam/) | KNOWN-DEVIATION | `source-frame-000` | 41158 | battery, footer, glyphs, body |
| [`journey-showcase-s01_home`](journey-showcase-s01_home/) | KNOWN-DEVIATION | `source-frame-000` | 41250 | battery, footer, glyphs, body |
| [`journey-showcase-s02_mix_comp`](journey-showcase-s02_mix_comp/) | KNOWN-DEVIATION | `source-frame-052` | 28239 | battery, footer, glyphs, body |
| [`journey-showcase-s03_filter_sweep`](journey-showcase-s03_filter_sweep/) | KNOWN-DEVIATION | `source-frame-052` | 36565 | battery, footer, glyphs, body |
| [`journey-showcase-s04_breakdown`](journey-showcase-s04_breakdown/) | KNOWN-DEVIATION | `source-frame-000` | 41251 | battery, footer, glyphs, body |
| [`journey-showcase-s05_909`](journey-showcase-s05_909/) | KNOWN-DEVIATION | `source-frame-000` | 41250 | battery, footer, glyphs, body |
| [`journey-showcase-s06_303a`](journey-showcase-s06_303a/) | KNOWN-DEVIATION | `source-frame-000` | 41250 | battery, footer, glyphs, body |
| [`journey-showcase-s07_break`](journey-showcase-s07_break/) | KNOWN-DEVIATION | `source-frame-000` | 41250 | battery, footer, glyphs, body |
| [`journey-showcase-s08_fx`](journey-showcase-s08_fx/) | KNOWN-DEVIATION | `source-frame-044` | 35490 | battery, footer, glyphs, body |
| [`journey-showcase-s09_mix`](journey-showcase-s09_mix/) | KNOWN-DEVIATION | `source-frame-052` | 28246 | battery, footer, glyphs, body |
| [`journey-stereo-s01_pans`](journey-stereo-s01_pans/) | KNOWN-DEVIATION | `source-frame-048` | 31044 | battery, footer, glyphs, body |
| [`journey-stereo-s02_panned`](journey-stereo-s02_panned/) | KNOWN-DEVIATION | `source-frame-048` | 32808 | battery, footer, glyphs, body |
| [`journey-stereo-s03_bd_pan`](journey-stereo-s03_bd_pan/) | KNOWN-DEVIATION | `source-frame-049` | 39937 | battery, footer, glyphs, body |

## Golden approval log

- 2026-11-16T14:14+10:00 — approved by DnBOX Lead, baseline of 94197fc8 (records the firmware as it renders; not a design approval): "User 2026-11-16: "I want to understand why there's so much regressions" / "Make one thing, breaks all the other" / "Then pls fix"" — golden updated: 777 states (each listed with old/new sha256 in APPROVALS.tsv)
