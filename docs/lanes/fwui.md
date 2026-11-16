# Lane: fwui
Owner: Dot firmware UI lane. Branch: lane/fwui-r22. Base: e742be7bd0d01323c899de6e156f40b85ee451db. Target: integration/dnbox-consolidated.

## Checklist
All C1-C6 and F1-F7 remain FAIL (not yet qualified at this lane head).

## Now / next
- Now: F3 code-space fit and F2 real-render geometry, reusing existing consolidated UI/resource changes.
- Next: source-pinned baseline measurement, smallest safe correction, independent test-and-merge review.

## Blockers and routes
- Repo-prep v2 is not yet merged: consume attached v2 AGENTS/RULES/CHECKLISTS and design inputs without modifying prep-owned files; merge consolidated when available.
- Local Git store has missing ancestor objects despite a complete e742be7b checkout: verify current tree and obtain ancestry through connector; do not invent history.
- Current first-slice detector requires explicit --render-dir and --output; a bare invocation is not all-screen evidence. Capture the actual exact-source renderer closure before evaluating.

## Top 3 risks
1. Existing UI does not fit native code space with all engines.
2. First-slice detector does not cover all screen domains.
3. Shared ui.c changes require direct coordination with sound/integration.

## Deletes
None.

## Conflicts
Historical briefs are evidence only. RULES.md v2 from owner-provided dnbox-repo-prep.zip governs this lane; no inherited historical approval is treated as current instruction.
