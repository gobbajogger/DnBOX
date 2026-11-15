# DnBOX whole-instrument UI model bindings

Source-only preservation of the reviewed shared Settings host/lifetime and source-native waveform modules. This is not guest firmware integration, an installable website, a complete UI design disposition or a qualified device build. The original design baseline remains in the parent branch; these files do not replace it.

The integrated local design candidate has SHA-256 `05cf1e7ce46a2efa630955375192b71215bc8741baa808bbd17f0c1619661d6b`. Its underlying visual firmware reference is `3c41ef4bedf9701f549ce2752b73306e2fab1eb8`. The HTML, fonts, runtime packages, native captures and other private input payloads are deliberately not included. The modules are an overlay, not a self-contained application. Use only a separately authorized exact design-model input when reproducing.

- `settings-host.js`: typed four-PERFORM current-host adapter, explicit units, guards, input epochs and coupled history; closes restore caller UI only.
- `settings-ui.js`: per-session iframe/channel ownership, refresh invalidation, stale-load protection and epoch-guarded return. No timeout-based correctness.
- `frame-client-lifecycle.js` and `frame-transport-lifecycle.js`: acknowledgement-before-retirement channel protocol and idempotent host close.
- `settings-native-skin.js`: source-family layout and explicit unsupported context/navigation/power/dirty states.
- `source-wave.js`: exact existing Chicago glyph raster/cache interface. A later improved morph proposal is separate and is not asserted by this source freeze.

Controlled asynchronous Node MessageChannel checks cover cancel-before-load, reversed frame loads, delayed old retirement and fresh-session close. The two stale-continuation regressions reproduced against the prior frozen input and passed after the two-line epoch guard. These tests execute the actual supplied candidate scripts in an inert DOM with real Canvas/MessageChannels; they do not emulate browser security, scheduling, physical devices or audio. Broader prior model and unchanged current-firmware WASM observations are separate evidence, not acceptance of these modules on hardware.

Only four PERFORM scalar bindings are supported. Value-only CAS retains an ABA limitation. PLAY, clip, USB/clock-source mutation, native save/load/history/resource behavior, peer connectivity and capture/lifecycle interfaces remain unqualified or reserved. No merge, package/binary publication, device write or new runtime behavior accompanies this source preservation.
