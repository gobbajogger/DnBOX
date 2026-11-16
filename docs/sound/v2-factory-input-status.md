# Factory input audit, 2026-11-16

This is input integrity/identity evidence, **not** preset DSP admission, sound-lane
completion, or a legal conclusion about redistribution.

## SuperJ6: S5's 640 presets / 12 banks

Supplied `J6presets.zip` SHA256:
`ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8`.
The original files remain preserved locally. Deterministic gzip copies were
round-trip checked; no original parameter values were changed. All 640 presets
load, containing 90,549 parameter entries and 174 distinct identities. 172
identities occur in the recorded 218-identity source schema. Unknown identities:

- `neonlimiterAmount`: 240 occurrences.
- `neontubeRageAmount`: 34 occurrences.

Do not alias or discard these fields to make a coverage check pass. All original
bytes remain intact. Source identity recognition does not prove value domains,
order, default handling, FX execution, or audible DSP binding. S5 remains FAIL.

`tools/audit-sj6-factory.py` takes an explicit input folder, byte/hash manifest,
and source schema. It verifies both compressed and original bytes before parsing,
rejects duplicate parameter identities and nonfinite/non-numeric values, checks
bank/corpus counts, and reports unknown identifiers with a failing exit status.
Synthetic regression tests require no factory payload. Exact report is
`v2-superj6-factory-audit.json`; its manifest/schema hashes pin inputs.

Replay with the preserved supplied corpus and its generated manifest:

```
python tools/audit-sj6-factory.py --banks <preserved-gzip-banks> \
  --manifest <preserved-gzip-banks>/MANIFEST.json \
  --schema assets/sound-schema/sj6-page-audit.json \
  --out <audit-output.json>
python tests/host/test_sj6_factory_audit.py
```

Expected current audit exit: 1 (unknown identities), not a test infrastructure
failure. Seven synthetic regression tests pass, including corrupt input hashes,
missing bank, duplicate/escaping paths, duplicate identifiers and bad values.

### Publication scope

An automated safety review rejected the first attempted bank blob upload due to
unclear authorization to disclose a private-recovery payload to a public repo.
No bank blob was uploaded and no alternate route was attempted. All 12 JSON banks
were screened: 23 field names, no credential-named fields. The supplied archive
has no redistribution license. The official
[SuperJ6 page](https://audiokitpro.com/synthonej6/) names the included designer
banks but does not establish preset redistribution rights. The separate original
AudioKitSynthOne repository does not by itself establish rights for J6. Rights
remain unresolved, not proven prohibited; parent is handling the exact owner
clarification. This commit contains code, synthetic tests and metadata only,
not preset values or bank payloads.

## Synth One is a separate corpus

Official `AudioKit/AudioKitSynthOne` source pin
`6466a3715c96b7ecf1dd255a218cbd571408a314` contains 694 preset entries across
12 non-User JSON bank files. File bytes were recovered and each original Git
blob SHA verified. The legacy 56-program comparison only covers BankA+StarterBank,
not this full corpus. Its source-controlled `initBanks` and bonus insertion order
must drive factory enumeration; do not equate the SuperJ6 640/12 requirement with
Synth One. Full factory/frontend/FX qualification is still open.
