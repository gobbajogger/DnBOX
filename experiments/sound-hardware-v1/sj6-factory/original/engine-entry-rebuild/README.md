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

## Factory and component boundaries

Run probe_original_au_routes.py with the same --repo and --ipa-recovery
arguments as the initializer checkpoint. It reads the original Objective-C
class and method metadata and executes the two non-nil state entry paths
until their first import. The receiver and argument markers are never used as
working runtime objects. A successful probe records the import endpoints.

Run probe_sampler_authored_state.py with --repo, --banks J6presets.zip,
--bench-archive SuperJ6-bounded-reference-bench-20261115.private.zip, and
--bench-directory pointing to the extracted bench in the original component
replay layout. It authenticates the required bench members against the ZIP
and the running original AU bytes against the executable pin. It passes the
exact Medieval Sky filter-sustain field to the original sampler DSP and
core setter and checks both readbacks. This covers an individually applied
component value. Full factory recall and original stereo audio remain pending.

## Original live receiver prerequisite

Run probe_live_receiver_constructor.py with the same --repo and
--ipa-recovery arguments as the earlier initializer checkpoint. The probe
supplies the build-9 AudioComponentDescription and executes the original
Objective-C initializer and Swift body up to the first dereference of
the actual Mach-O binding for the Darwin stack guard. It executes the
extension factory up to its first Objective-C retain. The probe supplies no
stand-in guard or objects. Its internal nil-state test returns without work,
and its nonnil-state test stops before the first receiver lock access.

The constructor later assigns the receiver lock at +0x138 and model at
+0x68. Those are live runtime objects. The release machine code initializes
the lock using Objective-C allocation and initializer dispatch. The model
assignment follows a Darwin block copy/invocation path and Swift retain.
This static continuation is separate from the measured execution endpoint.

The same constructor probe also runs with the authenticated Darwin stack-guard slot resolved to private process data. The original prologue copies the bound value to its own stack. Two independent runs each executed 37 original instructions and reached a write to the live receiver's `engine` field at `0x1000bc7e4` and offset `0x60`. The probe records the fault before any object exists.

Chained bindings in the original executable identify the next dependencies. The local extension superclass requires `_OBJC_CLASS_$_AUAudioUnit` from AudioToolbox, and its cache and the subclass cache bind to `__objc_empty_cache` from the Darwin Objective-C runtime. The setter then requires a live receiver lock and Swift model. The [measured constructor paths and authenticated bindings](live-receiver-constructor.json) leave complete factory state application and stereo captures open.

## Engine-only extension factory after nil retain

Run `probe_extension_dispatch_boundary.py --au ORIGINAL_AU.zip` with the pinned Unicorn environment. The probe authenticates the original archive and executes the original extension factory directly. Its sole import return is the ABI-defined nil retain. Any nonnil argument or other imported symbol stops execution before the imported code runs. The nil-input run executes 31 original instructions and stops at `DispatchWorkItemFlags` metadata from Swift Dispatch. The non-object marker stops at the original retain after ten instructions. Both routes leave graph, preset and capture counts at zero. [Measured import endpoints](extension-dispatch-boundary.json)

The [input inventory](runtime-input-inventory.json) covers the five existing private assets, including the nested IPA member list. The IPA includes the original Disk framework. The inspected archives have no AudioToolbox, Darwin Objective-C, Foundation, SwiftCore or Swift Dispatch runtime images. A filesystem check of the selected workspace, Library mount, `/opt`, and `/usr/local` also found no redistribution of those runtime images or dyld shared cache. The original subclass and model traces require those runtime facilities before they can produce a live receiver and model.
