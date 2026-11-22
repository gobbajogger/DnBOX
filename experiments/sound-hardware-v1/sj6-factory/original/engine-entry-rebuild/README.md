# SuperJ6 Linux initializer checkpoint

The checkpoint authenticates the recovered build-9 IPA, Mach-O and Medieval Sky factory stimulus. It executes only the original initializer instructions before their first linked Apple framework call. The complete instrument graph, original audio capture, and S4 scoring are pending.

The independent unit test image is synthetic ARM64. It verifies forward and backward branch decoding, Mach-O stub/name lookup, actual Unicorn execution to the imported boundary, and refusal when the pinned input is absent. A separate synthetic bank countertest checks the release/import container distinction. No original executable or sample is included here.

## Replay

Use Python 3.12 and Unicorn 2.1.4 from an isolated package directory.

```sh
python3 -m pip install --no-deps --target /tmp/sj6-uc unicorn==2.1.4
export PYTHONPATH=/tmp/sj6-uc
python3 -B -m unittest -v
python3 -B rebuild_engine_entry.py --repo /path/to/DnBOX \
  --ipa-recovery /path/to/com.audiokitpro.superj6-0.999.8-Decrypted.ipa.zip
```

The outer ZIP is pinned to 56,892,747 bytes by its known SHA-256 `3cc86adee1b4b8a0516306f02d6ffeb2e828144565a824e75624212998366592`. Its sole non-metadata IPA member must be 57,135,325 bytes and match `dde4992f24f52d4390391069025b27092a2f327e3a53115bdd53a6ede0dffa8e`. The embedded AU SHA-256 is `a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1`. The checkpoint invokes the recorded release matrix selector and validates the 640 factory records, 90,549 control fields, and separate release-only User Bank using the archived 12-bank audit. The separate export may instead be supplied via `--au ORIGINAL-AU.zip --banks J6presets.zip`. That route labels the stimulus with the export record hash.

Both missing inputs and a reached framework checkpoint exit 2 with a structured status and zero qualifying captures. A wrong digest, changed executable, unsupported layout, wrong program or unexpected execution path fails closed with an error. The script neither installs nor launches the app. It decodes the first ARM64 branch target from the executable and resolves the linked name by the Mach-O stub tables. The known original boundary is `KoFMInstrument` initializer `0x100059aec`, call site `0x100059b14`, import `_$s10Foundation4UUIDVMa`. Framework calls are never emulated as successful by this checkpoint.

The real recovered IPA verified the exact factory corpus, selected Medieval Sky, and executed 11 original instructions in Unicorn. The dependency is the first Foundation call at `0x100059b14`. The Mach-O stub at `0x1005d762c` loads the authenticated chained bind through `0x100733aa8`, rather than a live indirect-symbol entry. That bind resolves to `_$s10Foundation4UUIDVMa`. The command exits 2 with `BLOCKED_AT_REAL_FRAMEWORK_IMPORT` before returning from that import. No sound was rendered.

To reproduce the direct-constructor endpoints, run the same command with
probe_direct_constructor.py in place of rebuild_engine_entry.py. It steps the
first metadata helper and the lower initializer separately, exiting 2 at their
own Swift and Dispatch imports. The lower initializer receives a mapped
placeholder address. It has no live Objective-C receiver or applied preset.
The alternate AU/export ZIP route was also exercised and stopped at the same
Foundation import. Its canonical authored record hash differs from the
canonical bundled IPA record hash, and each route validates its own identity.
