# Reference identity audit

S4 names a SuperJ6 “14/56 comparison ZIP”. This exact SuperJ6 input has not been identified. The earlier lane assertion that it was available was unsupported and is corrected. No other archive is substituted.

The verified artifact with an explicit 14/56 result is `SynthOne-original-preset-musical-comparison-20261115.private.zip`, 32,977,854 bytes, SHA256 `67398be90eca2b20cc82b9d6e56b3c36b9a5ea04980ce7714c4e50d5e2b01cb4`. Its `source-and-evidence/HEADROOM-AND-AUDIO-EVIDENCE.private.json` SHA256 is `f23974d80e96dcd05375c1c7e172ba624d352bfc484bbdbef538b020234ae52b`. It contains 56 Synth One presets, 14 over unity and a worst peak of 3.2324109077453613. This was raw-versus-compact with common authentic FX, not the Apple app or independently implemented target FX. Over-unity float output is not itself proof of device clipping.

Read-only Library recovery also preserved four separately named SuperJ6 reference archives. Their distinct scopes include selected component tests and explicit failed word-identity tests; none proves the requested complete 64-control graph or identifies the 14/56 ZIP. Source search and archive-name matching are not recovery of a differently specified input.

S5 remains incomplete: 172 of 174 stored identifiers have verified mappings. Static original AU metadata names ordinal 17 `limiterAmount`, while verified serialized ID17 is `neoncompressorAmount`. That mismatch does not establish a legacy alias for `neonlimiterAmount`. Literal absence of the full prefixed name is also not proof of absent behavior, since identifiers can be composed. `neontubeRageAmount` remains unmapped. Preserve both unknown identifiers and fail closed, rather than dropping them or inventing aliases.

No original binary, factory bank, model coefficient or PCM payload is included in this audit. No design/rule change or completed checklist item is claimed.
