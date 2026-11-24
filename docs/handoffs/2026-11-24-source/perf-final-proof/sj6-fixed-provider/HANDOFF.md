# Immutable export-order SuperJ6 host provider (not installed)

Exact canonical input: `composed-pat-wire.UkREYb/POST.sha256`, digest
`f7d17a572dc1affdde651cb8764857da92ddb6308a3194d2bb72618ecbf98c12`.
All 314 files verified before a private copy. No checkout, device, or product changes.

v1 authority is the member-SHA-authenticated owner export, archive SHA-256
`ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8`.
Its ordered detached UUID/name/position/MIDI identity manifest SHA-256 is
`e028d9fd9b1d7715f4402af76334f7475692d18406b58c14e4d89413a6ea9931`.
Native Build9 AU/bundle metadata is NOT a fallback: the independent pinned audit
found matching program UIDs and 90,549 exact ordered Float32 words but different
bank UUIDs in all 12 banks and different positions for 525 programs. Neither
same patch words, same label, nor occupant of a position authenticates the
`(catalog namespace, ordinal, bank UUID, program UUID)` tuple.

- Host-only format bounded to exactly 496,414 bytes, 640 rows, 12 banks, each
  UTF-8 label up to 92 bytes and each row up to 240 authored words. Host-only
  OpenSSL SHA-256 over an owned snapshot, compared to a trust pin compiled
  outside the payload. Validate header, row positions, bank transitions, and
  exact trailing length before exposing any record. No mutable borrow from
  input; failed reopen revokes access. Trust requires pin provisioning by a
  trusted release process, not a hash provided beside unknown bytes.
- The loader traverses the authenticated original ordered ID/binary32 fields
  into the existing `sj6_factory_add_word` and
  `engine_superj6_factory_load`, drained through the actual production
  render path. No mock staging accepted. Same registry, same receiver, same
  storage/busy holds. Fields that this factory row API does not translate
  remain unimplemented; counting exact field transit is not control, audio
  parity, or shipping coverage. Stereo proof is representative audible
  output from the actual initialized DSP pointer for each owner, not Build9
  waveform parity, SMK MIDI, or physical playback.
- Host acceptance loads 640 x 2 through the real API; compares all 90,549
  IDs/order/words with the separate compressed C bank reader by UUID; sees
  each bank/program label exactly; checks peer cells unchanged and a stereo
  rendering on each owner from each of 12 banks. Legacy four indices still
  use legacy selection. Rejected input and allowed auditions never touch
  current/previous project objects, lengths, or write count, and actual
  legacy reload still restores the prior sound. Audition modifies the live
  compatible sound intentionally via existing compact owner API.
- Negatives: absent/short, tampered magic/version/count, ordinal, bank and
  program ID, first label, authored data; wrong valid bank/UID as arguments;
  real records rearranged and made structurally ordinal/position-valid;
  rewritten complete valid bank UUID without changing accompanying words.
  Caller-side modification after opening cannot substitute identity.

Existing SOUND/PAT synthetic wire is deliberately not connected. Real 640
on-unit recall requires storage and trusted immutable provider, verified
typed reads/saves/per-pattern restores, same meaning across owners/boots,
and a genuine UI provider. This oversized analytic capsule (+2,560 B host
index, no streaming flash budget asserted) is not a flash packing proposal.
It is not a production ABI, physical storage allocation, parser/panel
integration, flash fit, release artifact, or website enablement.

## Reproduce

With this workspace as W, set FIXED_SCRATCH to a fresh private directory;
copy the exact verified canonical `source` subdirectory there. Generate
`catalog.bin` via `stage_fixed.py FACTORY OWNER_EXPORT FIXED_SCRATCH/catalog.bin`.
Generate `picker_catalog.inc` via the prior
`sj6-picker-host-hook/stage_picker_catalog.py` with identical first two
args and that private output path. The compressed independent C reader
fixture is `sj6-private-final/bench-work/compiled/bank_data.inc`, produced
by the pinned `compile_bank.py`. Run `sh replay.sh "$W"` with the same
FIXED_SCRATCH. Only source, test commands, checksums and aggregate results
are in this packet, not the private payload.
