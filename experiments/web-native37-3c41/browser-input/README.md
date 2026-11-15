# Native37 counted browser-input ownership

Fresh isolated candidate. Derived from accepted #899 controls.js heldRouter/momentary behavior at commit431d0db82cbadca761fb32c6548ca4765875324f, verified through connected GitHub; upstream blob/provenance and complete MIT copyright/license retained. NOT a firmware model, live WASM binding, browser acceptance result or device implementation.

## Integration

```js
const router = heldRouter((kind,id,value) => send(kind,id,value));
const cancellation = bindCancellation(router,{win:window,doc:document});
const mount = mountMomentary(button,{router,kind:'physical',getId:()=>selectedPhysicalId});
// pads use kind:'pad', getId:()=>padId
// Before host reset, boot transition, stop or teardown:
cancellation.reset();
// On removed DOM controls:
mount.destroy();
// On surface teardown:
cancellation.destroy();
```

Physical IDs0..36 produce `send('physical',id,100)` on first hold and velocity0 on last release. Pad IDs0..15 produce1/0. These are physical identities, NOT MIDI-note numbers: no upstream MIDI_BASE48 mapping is copied into the native C path.

Each pointerId, Space and Enter has its own source token. Two pointers, two keyboard activation keys or mixed sources can hold one control; first down/last up only. Repeats are idempotent. Selection is sampled only on a fresh press; release always owns the original captured target. Pointerup/cancel/lostcapture release the matching pointer only; local element blur releases keyboard tokens only. Window blur or document hidden releases all tokens and updates every mounted aria-pressed state. Visibility regain never replays notes. Mount reset/destroy releases only that mount, while surface reset releases all. Nonprimary mouse buttons and disabled/repeated activations are ignored; releases still work after disabling. Pointer capture failure is caught, but automatic off-element release without capture is not proven; global cancellation remains available.

The injected send callback must preserve command order and handle transport errors/rejections. This module tracks browser intent, not acknowledged C state, and does not await/qualify host delivery. The parent owns C event transport/queue and genuine observed-state feedback. Do not color browser-held state as confirmed engine gate.

## Verification

`node --test input.test.mjs`

10/10 behavior tests observed RED with no-op API, then10/10 GREEN. Tests execute the actual JS router and actual installed listener callbacks on deterministic EventTarget objects, with a tiny attribute/capture shim. They do not launch a browser, generate trusted pointer events, prove browser capture/focus behavior, produce audio or exercise C/WASM/device state. Tests cover mixedsource releases bothorders, two pointers, Space/Enter, changing selection, repeat, cancel/lostcapture, local/global blur, hidden/reset, teardown and invalid input.

Source-only candidate pending existing independent reviewer. No parent native C/Python/browser-shell or frozen picker files changed. No browser-origin retry, publication, Git push or device action. Requested model/effort not independently exposed.
