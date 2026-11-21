# Original SuperJ6 reference bench

## Linux replay

The original AU executable has SHA-256 `a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1`. The Linux path uses the November 15 bounded ARM64 runners and their original component entry points with Unicorn 2.1.4. The archive reader uses the AU zip and supplies the same checked Mach-O bytes directly. The standalone app never runs. Use the private inputs named below and a new output directory.

```sh
python replay_reference.py \
  --au SuperJ6-AU-original-extension.private.zip \
  --bench SuperJ6-bounded-reference-bench-20261115.private.zip \
  --matrix SuperJ6-real-asset-musical-loop-matrix-20261115.private.zip \
  --out replay
python graph_entry.py replay
```

Five chorus cases, a CoreSynth note and a CoreSampler note produced seven byte-identical stereo PCM files, 4,096 frames each at 48 kHz. The seven file hashes are in `component-replay.json`. Running the KoFMInstrument initialization prefix reached the original framework call for Swift Foundation UUID metadata before any factory preset loaded. Its next calls construct a UUID and obtain a UUID string, then allocate a core Objective-C object. The isolated DSP bridges do not provide these framework objects or the AU audio graph. `instrument-init.json` records the executed call site and exact import. The Linux package contains no complete preset audio.

## Factory inputs

The [release matrix selector](../release_matrix.py) takes the build-9 IPA or installed app bundle and emits the 14 complete original bank records, stimulus and audit for the headless host. The [bench commands](../README.md#s4-input-and-scoring) use this input route on both arms. The archived comparison below used an independent bank export. Generate those export records with the sibling `factory.py`, then run the release audit on the original IPA as data only. Each selected UID has 1 unique same-name-bank entry in that release. All 1,926 ordered control values match binary32. The release bank and exported bank have different UUIDs in all 12 banks. Four volume values have alternate decimal spellings with the same float32 words. Two records differ in non-control metadata. The audit retains both bank UUIDs, each input digest, and those differences.

```sh
python ../factory.py J6presets.zip matrix /tmp/sj6-selected
python audit_bundled_banks.py com.audiokitpro.superj6-0.999.8-Decrypted.ipa \
  /tmp/sj6-selected > /tmp/sj6-selected/bundled-bank-audit.json
```

## Engine-only AUv3 host

The Swift package hosts the registered original instrument through Audio Toolbox. It requests a new AU for each program, checks its record and live parameter readback, and calls the AU render block directly. Events use the 44,100 Hz frame clock in `s4-inputs.json`. The host requests stereo Float32 at unity and supplies no external transport/clock. Output is 132,300 frames per preset. It never requests the AU view controller or starts the standalone SuperJ6 app.

On a compatible Apple host that already has the matching AU registered, generate the inputs from the installed `.app` or the IPA, then copy the JSON files to `apple-host/Tests/ReferenceBenchTests/Inputs/`. Run the package test on that host in Xcode. Its `xcresult` holds the UID-named raw PCM, state readbacks and JSON receipts. Export the attachments using their original names before running the sibling `compare.py` on paired original and firmware captures.

The nil host tempo and transport callbacks leave the instrument on its authored clock. The host reads back seqTempo and schedules the first note at absolute sample zero before calling the first render at sample zero. The receipt includes both times. The AU's build-9 instrument class exposes `factoryPresets`, `setCurrentPreset:`, `fullState` and `setFullState:`. The host tries the factory entries returned by the plugin, then its document-state loader. It checks `lastLoadedPreset` and `appStatePreset` and compares every authored Float32 value against active state and live AU parameter readback. Unknown names, transformed units and linked setters have no automatic alias. Missing/different active or live values leave a diagnostic PCM and fail that row. Two historical identifiers, neonlimiterAmount and neontubeRageAmount, have no source-verified current name. When absent from both enumerated trees, the native factory loader may qualify them only with matching active snapshot values. The document-state route remains diagnostic in that case. A missing preset identity, silent output or nonfinite value fails that program. macOS includes a registered component URL when available. The host compares the hash of its executable with the audited reference. Where no registered executable URL is visible, the receipt includes the release manifest digest and component version but omits the live AU digest. Its raw recording remains available for independent installed-image verification; the S4 scorer excludes unverified images.

The Swift package has not been compiled or executed with the Apple SDK in this Linux run. Verified work comprises the Linux PCM replay, engine initialization probe, audited release data, and source inspection of the AU entry points. No complete preset render is being claimed.

## All supplied banks against the release

`audit_all_programs.py RELEASE.ipa J6presets.zip` reproduces `all-programs.json`. All 640 common program UIDs have the same ordered 90,549 binary32 fields in aggregate. The 200 different JSON numbers all represent the same Float32 master volumes. 525 records differ in position and 25 in originalUid. The separate release-only User Bank contains Your Init Preset. Loading and applying a program on DnBOX remains a separate test from this data comparison.
