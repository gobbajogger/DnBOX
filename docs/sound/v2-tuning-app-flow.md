# Original app tuning call-flow audit, not execution

Source: AudioKit/AudioKitSynthOne6466a3715c96b7ecf1dd255a218cbd571408a314 (MIT). Newly consulted exact Git blobs and observed code-search results are under tests/fixtures/synthone-tuning-app. No Swift compiler or original app execution occurred; the earlier download denial remains stopped.

Confirmed source distinctions:

- PresetDataManager applies optional master sets before the scalar parameter setter list when saveTuningWithPreset is enabled. This differs from the frozen factory bench's fixed tuning table. Original AudioKit math clamps generated values to its sample-rate Nyquist, so the runtime Nyquist admission is not itself evidence of a tuning mismatch.
- Tunings.swift tuningDidChange iterates `0..<127`, sending indices0..126, not127. The source kernel initializes all128 slots to12ET first. The prepared Swift reference produces all128 AudioKit table values, explicitly math-only; copying all128 into a claimed original-app state would require accounting for this callback distinction. No automatic bug fix or inferred audible app result is made.
- The documented master-A4 behavior in the upstream Tunings README assigns middleCFrequency. Exact code search returned only that README for middleCFrequency. The scalar kernel setter truncates/stores frequencyA4 but does not retune the table in that method. A complete executable callback route is not established. Do not invent an A4 retuning callback from documentation alone.
- setTuningTableNPO search returns seven declarations/definitions/forwarders, not a verified tuning-selection callsite. The original sequencer supports NPO, but this does not prove the app invokes its setter for a preset. Preserve this distinction rather than automatically equating computed table.npo with active DSP NPO.
- isHoldMode is parsed/stored in Preset.swift; code search finds that file and JSON banks, not an application use elsewhere. Its five nonzero stored values are not evidence that preset loading activates hold. This agrees with the earlier audit's explicit stored-field-only count. Live hold interaction still needs its own original-app evidence.

Search coverage is lexical, not a proof that indirect/reflective behavior is impossible. These are source-backed constraints on the next original-capable test, not a substitute for Swift/app execution or permission to change original semantics. Preserve the original index127, A4 and NPO behavior until actual source/runtime evidence settles it. No production DSP or current frozen audio inputs change in this audit.
