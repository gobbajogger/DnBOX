# Same-candidate DNR3 digital SMK pad ingress to current Remote consumers

Website #206 current ac9001e01566fedebb0cc301f3a2090d079c3a06, open/unmerged;
base integration/dnbox-consolidated 93fbcdea3c12630a449195c01af923955ee6c535.
Actual PR app, link and wire protocol files match remote Git blobs
24cf61a06cd2306f548fc7b04e73438bf380582c,
048c46f619acf65bbb612e3fe123de20529a2fe1,
7f1c8ac3da15976cd68eeeaf6c80ae360073aeed. Base has no dnr3-link.mjs.
Firmware donor is isolated guarded whole-app host candidate, not shipping
integration; precise source files pinned here. It has settled Init-only
Synth One/SuperJ6 providers. No production changes to either repository.

True digital source path: diagnostic packet H is fixture stimulus only,
does not use the external MIDI or DNR3 note input. It calls extracted exact
firmware smk__padq with hardware pad index and post-ADC velocity, then
bridges the resulting platform key bits and strike velocity to the standard
host firmware. Production UI, bass engine, VOICE event admission/emission,
actual framed DNR3 wire, current JS link and extracted exact app event
functions all run unmodified. Local physical pad index8 with velocity107
sounded Bass A note41, then exact app highlighted that key and cleared it on
release. A separate clock transport stimulus generated six actual running
POSITION events and stopped at 127; the pad itself never advanced position.
Node DOM adapter is intentionally not a browser or visual/layout test.

Scoped compatibility: unchanged product fixture node suite 217 pass 8 skip
0 fail; not the exact complete terminal gate. Existing sandboxed browser
gate failed before navigation because Chromium executable was missing.
Current runner has only the chromium wrapper, no Playwright executable;
no bypassed sandbox and no browser install attempted in this slice.
No ADC sampling/threshold, physical pad hardware, actual browser/USB or
combined release coverage. Do not merge or deploy on the basis of this
evidence; same pinned native/browser ingress remains open.

Run: bash run.sh /path/to/guarded-owner-host. No code, bank or actual
audio included in the source-only checkpoint.
