# F7 category navigation proposal, not implementation

Source schema: sound lane 33d1c0514b51f3391b211d86a8653da5cd6765a3.
Existing UI reviewed at fwui 893db46d1c5b40a20d1224e79ce4b8262fea4071.

No owner approval or runtime admission is implied. No live control remapping has been made.

## Navigation candidate

For the new engines' PART pages only, preserve the existing four controls, SELECT page browsing and SEL deep parameter list. Add a CATEGORIES action at the top of that list. SEL on this action opens an index; SELECT chooses a category; SEL selects it and returns to its first four-control page. HOME returns from the category index to the prior parameter list; HOME from the parameter list closes it, as now. The normal parameter list retains ALGORITHM and arrow value editing. Keep SOUND engine/preset access as a separately reachable category. Do not change SEQ held-step chance, HOME+SELECT tempo, help, other engines or the pending F6 SEL+SELECT decision.

The category index is bounded: 25 SynthOne source categories plus SOUND, or 8 SuperJ6 groups plus SOUND. The page cache holds only the selected category, at most four pages rather than a 38-page flat SynthOne table. Existing MAXPAGES=14 and LIST_MAX=64 need not expand. Actual compiled footprint and navigation tests are still required, not projected PASS.

SuperJ6's eight source-audit proposal groups are Mix/Oscillator; Wave detail; Amplitude envelopes; Filter; Filter envelope/layer1; LFO1; LFO2; LFO3. Each has eight IDs, split into two four-control pages. This preserves all 64 metadata controls but is not proof of DSP binding or approved grouping.

SynthOne grouping is in synthone-categories.json. build.py checks exactly one occurrence of every ID 0..149. Sound independently checked identities; detuningMultiplier affects oscillators/sub/FM and belongs to global tuning. Envelope keyboard tracking is not a pitch source. Sequence IDs 67..82 are relative offsets (-12..12, scaled for non-12-EDO), 83..98 octave-boost booleans and 99..114 note-enable booleans. They are three groups of sixteen step fields, not new project storage or absolute MIDI-note controls.

## Unresolved qualification

Many source descriptor units explicitly require setter/formatter research. Hz/s/BPM metadata cannot authorize guessed physical formatting for other controls. Compressor makeup is a linear multiplier, not stored dB; source thresholds and timing display need source-grounded formatting. Dependent rate formatting changes with tempo sync. Unknown laws must remain visibly unqualified in evidence and cannot satisfy F7's real units requirement.

Owner decision is required for the category navigation/grouping design. No input/UI implementation occurs before that decision. Runtime source binding and native budget qualification remain separate gates.

Sound's source-semantic cross-check confirms source_fx.hpp multiplies cutoff by delayInputCutoffTrackingRatio (dimensionless, not the upstream Hertz tag); fx_compressor.hpp multiplies output by makeup values 134/135/136 (linear gain). The native metadata row at position136 incorrectly repeats the master makeup identifier; retain the enum's compressorReverbWetMakeupGain identity and preserve the original metadata only as provenance. Morph offsets3/4 have explicit RelativeSemiTones metadata. synthone-native-units.json records all150 upstream rows with source line/hash and warnings; it is not a validated display table.
