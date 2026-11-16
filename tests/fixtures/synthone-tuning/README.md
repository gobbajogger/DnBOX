# Exact original tuning-source research inputs

`SOURCE-MANIFEST.json` pins every retained original file to its public Git blob
and SHA256. SynthOne source is6466a3715c96b7ecf1dd255a218cbd571408a314;
AudioKit v4.9.2 resolves to03fecf80a66be88f6ac0356cd532c72dbe09922d.
The SynthOne Podfile.lock at blob b333d0d5a13e7e906fd08c82562f272d65dbf605
specifies AudioKit4.9.2. Both original MIT notices are retained. No installer,
SDK/toolchain binary or executable artifact is included.

These are unmodified research inputs, not a compiled replacement or an
independent app-audio oracle. Swift is not installed in the present host;
no Swift/original-iOS execution is claimed. The available Mac is offline; an
existing managed task is checking Swift availability after its current review. Do not add these files to the C++
factory runner's qualified closure silently or reinterpret frozen694 results.

Confirmed source flow:
- AppSettings defaults saveTuningWithPreset to true.
- PresetDataManager.swift (retained separately in the factory fixture) restores
  the saved master set or default tuning before scalar parameter restoration.
- Tunings.swift feeds master frequencies into AudioKit's tuning table, then
  copies frequencies into the synth and sends a tuningDidChange notification.
- Its actual copy loop is0..<127, not0..<128. Preserve/document that difference;
  do not silently repair source behavior and call it original equivalence.
- AKTuningTable octave-reduces and sorts the master frequencies, calculates
  note-dependent octaves around middleCNoteNumber60, clamps to its Nyquist, and
  stores Double frequencies. AKSynthOne converts to Float at the DSP bridge.
- NPO setters exist in the protocol/Conductor/Swift bridge/kernel. Full call-site
  routing and complete original-app A4 retuning behavior are still under audit.
  A descriptive README statement is not proof of an executed callback.

The original corpus contains12 non-12ET master sets,5 non440 A4 entries,5 nonzero stored hold flags and37 nondefault modwheel routes. These stored inputs are not qualified
by the fixed-tuning source bench. Run tools/audit-synthone-app-fields.py to
reproduce exact counts/identities from authenticated bank bytes. The numerical
12ET classification tolerates1e-12 after octave reduction solely for this audit,
not as an audio comparison threshold. No DSP threshold has been weakened.

Additional original keyboard/sustain/modwheel source is retained for separate
frontend routing research. Stored isHoldMode appears only in Preset declaration
and parsing in the searched6466 source, not its loadPreset callback. Nonzero
stored flags alone do not establish active hold behavior. Modwheel routes0/1/2
map to cutoff/LFO1-rate/LFO2-rate; original callbacks are preserved rather than
replaced with a generic modulation-amount control.
