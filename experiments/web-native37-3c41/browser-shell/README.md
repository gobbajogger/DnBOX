# Current37 native-host shell candidate

This directory is a source progression of `../../browser-shell/`, not a published site or browser-validated result. Only this staged directory was edited. Parent C/Python, root browser-shell and frozen `../browser-input/` were not changed.

## Required server contract

Do NOT attach this shell to the old27 host. The parent owns server switching after candidate review. It must serve this directory's index/app/style/assets plus the exact sibling import `../browser-input/input.mjs` (and associated MIT attribution). With this directory served as root, the relative import resolves to `/browser-input/input.mjs`; configure an explicit supported static route to the reviewed frozen module, not arbitrary filesystem traversal. No standalone development server script is provided here, and this task did not start one.

`POST /native-event` retains the existing same-host API/header contract. Required commands: `physical,id0..36,velocity100/0`, `pad,id0..15,1/0`, existing fixture0..19/release/snapshot/tick/button/encoder. Parent send queue preserves request order. Each accepted frame MUST include exact C-owned `physical.held` (integer0..2^37−1), `physical.padHeld` (integer0..65535), existing state.gridPadAvailable and native27 LEDs. Missing/invalid physical fields fail rather than inventing state. The old server remains incompatible until parent switches it; no browser result is claimed.

## Ownership and feedback

`surface.mjs` imports the unchanged #899-derived counted ownership module. All37 direct SVG keys are focusable rolebuttons with accurate white/black geometry and physical names;22white/15black. All37 selection options are enabled.16pads are real focusable buttons in two rows of eight, guarded by source-selected grid availability. Master/fader/connected-host paths remain unimplemented and labelled.

Pointer/Space/Enter sources retain original captured physical/pad identity. Direct control and selector controls share reference counts. Fixture load/lesson reset releases all sources AND sends host release before changing fixture. Failed host release rejects fixture load. Global blur/hidden releases all; individual cancellation/lostcapture/localblur retains frozen router semantics. Pad release remains enabled when mode changes and new pad presses are disabled.

Amber key/pad classes and real aria-pressed values derive only from acknowledged C held/padHeld fields.37-bit feedback uses BigInt, not truncating32-bit shifts. Frozen helper's intent aria is redirected to diagnostic data-input-held; it is not displayed as engine acknowledgement. LED green remains exact existing native27 bits: no outer light is inferred. C physical hold is not proof that every key produces a sound/action in every native mode, and not device feedback.

The original actual-current3c41 screen geometry, native integer-scale controls and smooth source-derived outlines are retained. No proposed new waveform glyphs or firmware UI replaced the reference. Main links/boot limits are copied, not republished.

## Proof and limits

`node --test surface.test.mjs ../browser-input/input.test.mjs`

20/20 PASS:10 frozen router tests +10 new shell-wiring tests. Nine new behavior tests were observed RED against a no-op surface before implementation; an additional failed-release regression was observed RED against the first implementation, then fixed. `node --check app.js` PASS. Tests execute actual JS routing and installed event callbacks against deterministic EventTarget/attribute shims. They are NOT trusted browser events, actual SVG focus/capture/layout/DOM pixels, C integration, audio, WASM or device proof.

Actual browser validation: NOT_RUN. No origin retry, Chromium/fileURI, browser launch, server action, publication or device operation. Parent will review exact candidate, then independently qualify C/API/shell and any permitted preview route. Git NOT PUSHED; PR none.

## Changed-input lifecycle correction

A fixture/reset transition now disables direct, selector and lesson admission synchronously, and remains disabled across every awaited host request. Fresh C response updates cannot re-enable controls mid-transition. The transition clears original router owners, awaits host release, then selects the fixture. A concurrent second fixture transition is rejected. A failed reset clears the current acknowledgement and leaves controls disabled until a fresh valid response. Global blur/hide during a transition reuses that transition rather than inserting a second reset. This remains a single-client ephemeral host contract.

Disposal is immediate for new input and API calls, removes listeners/owners, waits for any prior transition to settle without allowing its fixture send after disposal, and sends final cleanup once. Repeated destroy returns its existing cleanup promise. The caller must await teardown before starting a replacement surface on the same native session; a stale disposed API cannot issue later input or cleanup. Actual browser event/capture timing remains NOT_RUN.

Three new tests were observed RED for the independent findings, then the full23 frozen-input/surface tests passed. They include an app-style deferred ordered release with installed listeners, update during reset, lesson input exclusion, disposed public methods, repeated destroy and destroy while a fixture release is pending. This is JavaScript/DOM-shim proof, not browser acceptance.

Lesson admission now also requires a current valid C acknowledgement, before the first response and after rejected reset. This matches the fail-closed direct controls. Two added RED→GREEN regressions plus all affected input/surface tests:25/25PASS. No browser execution.

Alternate Hold controls now synchronize aria-disabled after acknowledged reset/fixture completion. Panel taps always attempt original button-up after down response failure; both errors are preserved if release also fails.29affected Node tests and explicit transitive module closure pass. These are source/DOM-shim results, not browser proof.
