# DnBOX whole-instrument design

[Readable mobile PDF](r8/DnBOX-R8-mobile-comparisons.pdf) · [phone comparison PNG](r8/preview/gen-held-comparison.png) · [workflow notes PNG](r8/preview/gen-workflow-notes.png) · [interactive proposal](r8/workbench.html) · [whole design and open work](r8/WHOLE-INSTRUMENT-DESIGN.md). The PDF uses390pt-wide pages with15pt body text and native240×240 images.390px fit-width checks pass; actual iOS-device opening is untested.

R8 corrects R7's modeled Undo transaction loss at GEN focus/REC boundaries and restores native cyclic GATE/ACCENT/SLIDE toggle grammar. Direct GEN correction is retained from the earlier proposal, removing the SEQ-and-one-ARP detour. These prototype repairs preserve existing firmware behavior. All15 curated stills reuse exact R7 pixels; corrected R8 interactions and three new native gate-state renders are separately pinned.

The exact successor passes35 focused producer model checks, seven retained DOM guides plus three native render checks, and135 mobile PDF checks. This is design WIP, **not completed whole-instrument acceptance or firmware implementation**. The same existing independent affected assessment remains pending. [Readiness and limits](r8/READINESS-AND-LIMITS.json) and [manifest](r8/ARTIFACT-MANIFEST.json) bind exact inputs.

R7 remains immutable at9b82d94dbc259cee7a00dcc6285522c36b8db3ba. Its review found meaningful WIP, partial fidelity and whole overhaul incomplete/not accepted. Its A4 PDF is a desktop/zoom reference. R8 does not replace its review seal or relabel unchanged proof. Current actual-before evidence is3c41/0.0.26 Elite; historical Settings25.x stays separate. No R8 firmware exists.

Full FX/controller, real connected use, populated BREAK, running/poly capture, durable recovery, reserved preset/held-note→capture strategy→lifecycle, Settings consistency, comparator adoption and whole audible musical acceptance remain unfinished. No build, merge, device operation or release follows from this source delivery.
