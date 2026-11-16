# Synth One replay artifact recovery status

## Verified and durable in Git

Source/test recovery inputs, licenses, recipes, original factory JSON, full
per-output manifest and receipts are on lane/sound-v2 at
e45460acb32ffbd16691d5a8c1b708c175e1c041. The independent managed environment
recreated all694 programs from that exact checkout, matching all1388 PCM byte
counts and SHA256 values. Its source input receipts match the producer's.
Neither environment used an SDK/toolchain binary or private microphone capture
as part of this portable host replay. Original source licenses remain included.

The canonical archive is533934080bytes, LFS SHA256 OID
`4e80387b4fc714c33b7239e27e0497c76c5cd15f2e496dbfaf45d2c709fdaa9e`.
Both environments verified archive readback against every expected PCM hash.
This demonstrates deterministic recovery, not complete sound-lane qualification.

## LFS transfer remains blocked

The managed authenticated object-only LFS upload exited2 with:

> This repository exceeded its LFS budget. The account responsible for the budget should increase it to restore access.

No alternate route or credential was attempted. Fresh-cache remote download was
NOT_RUN after the denial. A correct local LFS pointer/object does not establish
remote storage. **No dangling pointer was committed.** The producer originals,
managed recreation, archive, local LFS object and logs are preserved untouched.

Managed upload receipt SHA256:
`5ab638e6e2aa707512d715b83c3006e7f6073072396a4b13f89c36cc2594ff9a`.
Root owns the budget-access dependency. Sound owns retry/readback and pointer
publication only after access is legitimately restored. Do not split or reroute
this denied transfer to evade the repository budget. No upload success, durable
remote PCM delivery or deletion authorization is inferred from source recovery.
