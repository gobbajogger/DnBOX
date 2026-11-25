# Current implemented-controls source replay

This is the **implemented subset** of the original-engine control parity work,
not finished parity and not firmware to install. It adds no second LINK or
other invented control; it keeps the existing one FILTER LINK on both
engines, uses their original control semantics for the qualified scope, and
keeps factory presets out of the active development build.

1. Obtain the exact composed f7d17a57 firmware base by the preceding
   source handoff (firmware source manifest
   `../perf-final-proof/composed-pat-wire/POST.sha256`). Do not apply this
   overlay directly to the earlier released test or to main.
2. In a separate source checkout apply
   `patch --batch --fuzz=0 --no-backup-if-mismatch -p1 < implemented-controls-aggregate.patch`.
   SHA-256 of patch: 1d8e7190026592b0216b36215f3753ff497559ec68375acbd688420caf174dd0.
3. Verify `sha256sum -c MOD-WHEELS-POST.sha256` in the source checkout.
   SHA-256 of ledger b71f366f8da6ed85a64dbd3ba90b4c2784503511a01558c81ead2ee53e037e3d.
   A separate clean full apply matched every firmware path and byte plus
   tools/build.py and host/x0x_host.c. The overlay contains 21 source files,
   not generated binaries, private inputs, or test-only instrumentation.
4. Use the exact pinned tools and familiar combined flags:
   X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1
   X0X_SYNTHONE_PATCH=1 X0X_ARP_HOOKS=1. Leave
   X0X_SYNTH_CONTROLS_INIT_ONLY unset for the new default on Elite/Pro.
   For host fixture C direct compilation use -DX0X_SYNTH_CONTROLS_INIT_ONLY=1.
   No native build/profile/gate is altered merely by this replay.
5. Individual read-only harnesses, old-reader and focused host fixtures live
   beside their corresponding handoffs; use the exact inputs and frame type
   claimed by each. Latest paired physical MOD fixtures use the actual
   smk_wheels code and MIDI queue, with original INIT default destinations
   and held owner. They are not measurements on physical keyboards.

Do not apply add-on diagnostics (AMP-link experiments, unqualified vNext
storage, host-only receiver experiments, or read-only native flash
experiments) as product code. Alternate MOD targets and other original
controls still require source verification and end-to-end implementation.
Native fit, no-dropouts, protected legacy storage, and actual release remain
unqualified. No package, hardware write, or publication.

## Most recent implemented increment
After the exact b71f366f replay above, apply `../s1-init-reverb-lfo/s1-init-reverb-lfo.patch`
with zero fuzz and no backups. Verify `S1-REVERB-LFO-POST.sha256` (ledger
SHA-256 f74afed6ce7fc9087c4b1e3a8b2e53a3e5e5e2ad57fc1ea878ce5c49399b077b).
This adds the original Synth One Reverb Mix LFO in the existing FX
SWITCHES K3. No added page, second LINK or preset payload.
The previously unused 15-byte code states preserve prior valid bytes.
The earlier firmware refuses the new active state without writing.
This is the current host-qualified implementation, not a native release.
