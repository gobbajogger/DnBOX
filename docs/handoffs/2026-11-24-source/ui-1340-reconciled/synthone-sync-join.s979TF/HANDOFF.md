# Synth One oscillator editors on PWM2 and compact sync

The source-only patch joins the nine oscillator, sub, FM and noise controls to the exact PWM2 + compact SuperJ6 sync + optional owner-pointer checkpoint. It changes ten product files. Neither project format nor SuperJ6 DSP source changes. The two added host fixtures are shipped separately from the source patch.

## Storage crosswalk

| Address or route | Owner and joined behavior |
|---|---|
| `sound.v[T_303][0–1]` | A/B base Sound cells. The bank byte identifies the engine independently of the edit mask. |
| `sound.v[T_303][2–3]` | A/B shared ARP. Both owners were edited, saved and reloaded. These rows stayed byte-exact during the Synth One edit. |
| `sound.v[T_303][4–5]` | A/B SuperJ6 envelope/rate/authored-clock extension rows. Synth One edits and replacement leave the peer's base cell and extension byte-exact. |
| `sound.v[T_303][6–7]` | A/B Synth One extension. Its first 11 bytes store a 9-bit authored-position mask and nine positions. A marked Synth One bank owns this payload. An occupied unmarked payload is preserved and a new edit is refused until the bytes are available. |
| Edit-mask bit 22 | SuperJ6 PWM1 uses this bit through oscillator slot 4 at shift 18. Synth One uses the same bit to mark its own extension. Row validation, preset/reset cleanup and UI undo recognition qualify the mark by the Sound bank. The original port failed the mixed-bank predicate in `seam-red.log`. |
| SuperJ6 sync and PWM2 route | SuperJ6 sync uses its envelope/rate extension rows and bit 25, with authored words marked at bit 26. The existing LFO2 PWM packed byte is at cell byte 21 and its presence/PWM2 selection use edit bits 27/28. Playback and saved oscillator selection retain the pinned checkpoint semantics. |
| Note locks | Each owner has a separately tagged lane record. The existing 64-byte lane has a typed tail beginning at byte 49, with 11 bytes for marked Synth One or 15 for marked SuperJ6. In both orientations the locked onset recalls the captured morph and PWM2 depth, the following unlocked onset restores the edited bases, and a live Synth One write is busy while its lock owns the sound. |
| Motion and project data | Ordinary mixer-pan motion ran during the mixed test. The portable project validator rejected attempted normal motion targets at rows 6/7. Project save/load kept the full Sound and Arrange data byte-exact. |
| Engine changes and rollback | Synth One → SuperJ6 clears only the departing bank's owned extension. SuperJ6 → Synth One preserves opaque bytes and declines to edit into them. The exact previous reader loads unmarked mixed Sound records and leaves the opaque bytes intact. It rejects the edited Synth One Sound record as unsupported, refuses to save, and the durable-object bytes match after the attempt. |

## Compiled host evidence

- `seam-green.log` has 317 mixed assertions with both A/B engine assignments. The four held comparisons each change 4,096 samples in the edited owner's panned channel with zero changes in the peer. `seam-legacy.log` has 24 assertions with the unmodified checkpoint reader.
- `join-ui_synthone_oscillator_editor_test.log` has 194 assertions. All nine physical edits on both A/B change all 8,192 held samples in this fixture. Authored PCM and data survive reload, and INIT replacement restores the same PCM. `factory-editor.log` adds 28 assertions for factory program 129 identity and exact replacement on both owners.
- Existing host regressions pass for SuperJ6 sync (11,778), SuperJ6 PWM1/PWM2 (2,074), Synth One FILTER LINK (40), shared ARP (126) and the ARP SEQ keyboard (66).
- INIT and all 694 authenticated Synth One factory programs match dry/wet PCM byte-for-byte over 1,024 frames per preset. The four legacy bank paths match over 49,152 stereo frames. `factory-pcm.log` and `legacy-pcm.log` retain only digests. Private factory assets, PCM and stored project objects remain outside this packet.
- `panel/` contains eight direct 240×240 RGB host captures. The OSCILLATOR, OSC MIX and FM / NOISE pages show default and edited values. The mixed scene captures show the SuperJ6 PWM2 route and synchronized LFO2. Scope is focused host rendering.

## Exact reproduction

The preimage is the optional owner-check arm in [compact-fit/HANDOFF.md](../../perf-final-proof/sj6-lfo-sync/compact-fit/HANDOFF.md), staged for this run at `/tmp/dn-sync-owner.N4NpiD`. Reconstruct that preimage with the documented [PWM2 + compact-page chain](../../perf-final-proof/pwm1-lfo2-fit/page-matrix/HANDOFF.md), then apply [compact-sync-source.patch](../../perf-final-proof/sj6-lfo-sync/compact-fit/compact-sync-source.patch) followed by [control-owner-pointer.patch](../../perf-final-proof/pwm1-lfo2-fit/control-owner/control-owner-pointer.patch). Use the host-enabled donor with generated `build/gen/` inputs and the existing SuperJ6 sync/PWM2 test fixtures described in those handoffs. `PREIMAGE.sha256` and `POSTIMAGE.sha256` pin the touched source and adjacent integration files.

```bash
TMPDIR=/dev/shm bash replay-host.sh /path/to/exact/checkpoint /path/to/authenticated/s1.pack
```

The script verifies the manifests, copies the required sources into isolated temporary directories, applies the product patch to one side, and builds the paired host tests. Generated runs, fixtures and recordings remain under that private output directory. The patch also passed forward and reverse `git apply --check`.

The exact donor already overflows the stock Elite and Pro app regions by 13,100 and 13,092 bytes. Its disposable companion had 252 and 260 bytes of app margin with 512 above POOL reserve. The final joined target link, XIP neutrality and native emulator timing were outside this assigned host slot. Filter type and tracking are deferred pending measured product fit.
