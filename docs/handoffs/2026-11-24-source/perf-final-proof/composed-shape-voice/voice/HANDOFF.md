# DNR3 current-donor native source / host requalification

No application candidate, public refs, PRs, native builds, WASM emulation or
physical hardware changed or exercised. Product source patch and test packet
are separate. Keep BLE/controller/native packaging and release fenced.

## Identities and evidence boundaries

- Web review #206 current head is
  34127553a59dc91e3ce9c97581a434883349220a (connector exact review); the
  branch base is 93fbcdea3c12630a449195c01af923955ee6c535. Reviewed exact
  app, link and browser test source read-only. Local combined Web donor
  17f11860bf8c6c534f607938b7899319f2f89f33 tree
  f4662bcb5488eafc4a917f22ae0528ad2c0c2b95 includes e951625c-era native
  bass POSITION binding (actual commit e951625c79119f894bb6a71a92a649648dc4d0ab).
- Prior whole page device-pad evidence: Web 00deb8bc16391d926d25ec28c84143f6ef1e36e5
  + firmware 0aea5ab07730204abe106212715d1a4bc30f2fbf,
  web-final-data/voice-0aea5ab-e2e.log: 64/64 pad echo/independent real
  audio/VOICE cases across five parts. The overall prior test still had
  one distinct failure. The older 8eff report had accepted audio but no
  device-to-page VOICE and is a true negative, not positive evidence.
- Exact prior #206 BPM browser proof in the review's wasm-live-bpm-browser.py
  and web-final-data/bpm-206-proof/browser.log used adf3eee, kernel
  b16d487dd6b6b9ffe1dddcddc99df7e45fed6f28, wasm SHA
  92b81be4e2b8f8c59f34cdd94b7bb5cf28f2acf5f9350babb5ac5741c3729c65.
  It uses production Web MIDI, independent device STATE and TEMPO, lost
  outbound TEMPO write negative, preserved Draft value, and touch + keys.
  This is not native board identity proof: host/WASM HELLO board is 3;
  native HELLO is 1 for Elite, 2 for Pro. No native hardware simulation
  or proof for those specific models was requested/run in this task.
- Prior exact bass page proof in web-final-data/ux-native-bass-playhead.log
  shows page Play/Stop enables and clears a native bass marker, not native
  independent waveform/frame timing or position values. The older
  playhead-browser.py is explicitly Draft-only. No page-level native
  position range/same-revision/external start proof yet.
- Current full-control-candidate firmware snapshot is not a Git tree.
  All 313 source entries matched
  perf-final-proof/full-control-s1-wet-mix.LQZ92k/PRODUCT-POSTIMAGE.sha256
  (ledger SHA 518a2f2f09d1cebe5d4c49f7bb602c30af65edfffbdbf39ed86e462a0f2eda6d)
  before testing/applying. This donor already has DNR3 editor ingress,
  BPM and POSITION but not VOICE: caps 0x2ff, WATCH 0x5f (VOICE absent).

## Product delta and separately scoped proof

VOICE-forward-4files.patch is the clean 8eff93af→0aea5ab0 delta of
engine.c, original_runtime.c, remote_dnr3.c and breaks.c only. No Record,
Motion, controller, XIP/layout/font/pack change or standalone new flag. The
accepted physical/sounding hook uses actual owner paths across five parts
and emits only bounded lease-scoped latest notes; neither incoming MIDI
packets nor page sends are reflected as an optimistic echo. Both producer
and cache reuse are included. No physical device writes. Do not apply any
unrelated font relocation or reuse product metrics without measuring first.

- Pre 1322/0 on actual isolated donor host: an accepted physical engine
  command renders nonzero audio, but exact HELLO omits the capability,
  WATCH declines it, and no VOICE exists.
- Post 1341/0 on actual isolated patched donor host: all accepted five-part
  VOICE, bass release, physical no-packet witness, mute/unsubscribe/expiry,
  idempotency and the independent scoped tests. Tempo accepts precise
  137.5 BPM, STATE confirms it, panel changes to 141 BPM independently,
  stale epoch produces no change. A real Start advances five-track
  POSITION through multiple same-revision frames, immediate independent
  STATE matches, real Stop produces five 127 sentinels.
- Post real native BLE code path in host fixture: fragmented HCI ATT MIDI,
  HELLO/session/writes/long dump, WATCH, accepted VOICE, disconnect reset
  and wrong USB path isolation. Simulated radio only.
- XIP: no current candidate native build or size claim. Historical old
  117b4bbd with separate font relocation: Elite20/Pro32 B free, pool512.
  Historical 0aea cache change in that same campaign: 164/176 free, pool512
  (+144 XIP headroom). Exact current host DNR state shrinks 124→120 B
  with cache reuse; neither value measures total native SRAM. The current
  four-file patch deliberately excludes the old 380-byte font savings.
  Baseline exact and patched exact native Elite/Pro links and strict cold/
  all-six-offset sweep are separate mandatory gates.

Run replay-host.sh with current source, exact ledger, local firmware repo
containing the two pinned donor commits, and a fresh evidence location.
The recheck is test-only aside from the explicit patch inside disposable
snapshot. It requires no device, browser, network or package. The old USB
six-mutation matrix was not rerun in this focused requal. Post bodies must
progress CAPS/WATCH from absent to present, not force a mocked success.
Next functional UX test for new exact qualified runtime: independent MIDI
Start/Stop and active/queued pattern/offset step movement, both changing
same-revision POSITION and on-screen active part/cursor; separately inject
independent pad events and verify no false step light or disconnection
carryover. Run the existing BPM missing-ack proof again against the exact
composed runtime and two native-board package identities when allowed.
