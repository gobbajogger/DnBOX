# R9 input-priority corrections

With LIST open, PATCH+REC previously armed REC and PATCH release closed the list; PATCH+PLAY started transport instead of Redo. R9 routes these chords to bounded Undo/Redo while preserving LIST, transport, REC and original releases. BT plus a lower GEN knob now consumes the existing no-motion gesture without editing the generator or requesting SAVE on release. These are prototype fidelity repairs, not new firmware features.

[Interactive source](workbench.html) · [readiness and exact inputs](READINESS-AND-LIMITS.json) · [manifest](ARTIFACT-MANIFEST.json). The existing [mobile PDF](../r8/DnBOX-R8-mobile-comparisons.pdf), [native comparison](../r8/preview/gen-held-comparison.png) and [workflow notes](../r8/preview/gen-workflow-notes.png) retain their R8 identities. No new layout or visual execution is claimed. Actual iOS opening remains untested.

Exact final-HTML producer checks:14/14 LIST history,19/19 GEN SAVE-modifier and35/35 retained GEN transaction/control properties. Failed R8 traces remain separately pinned. Confirmation/clip masks and other-view motion clearing are preserved. Independent affected assessment is pending; whole-instrument design remains incomplete. No R9 firmware exists.

The HTML is self-contained. For reproducible local proposal checks, use Python with Playwright and an available Chromium, then run the corresponding check script with this directory and a separate output JSON path. `build_successor.py` uses the sibling exactR8 directory by default, or `DNBOX_R8_BASE`. The retained-scope check also needs the pinned siblingR7 source through `DNBOX_R7_SOURCE` and this directory through `DNBOX_R8_SOURCE`. These browser checks do not execute firmware or write storage.

Full FX/MIX/controller, valid populated BREAK, real connected behavior, recording strategy/lifecycle, durable recovery and reserved component interfaces still need meaningful journey design and verification. Existing source-derived actions, native glyphs, bright values, useful lower controls and real light meanings remain the basis.
