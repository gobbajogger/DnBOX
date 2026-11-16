# Partial host callback measurements: FAIL

These retained eight-program pilots do not cover all 694 programs. No source or runtime changes are admitted by these results.

| Run | Wall misses | Thread CPU misses | Nonfinite words |
|---|---:|---:|---:|
| Initial normal floating mode | 64 | 58 | 0 |
| FTZ/DAZ diagnostic | 29 | 27 | 0 |
| Same diagnostic executable, normal-mode repeat | 272 | 271 | 0 |

Every callback has 128 frames at 48 kHz. A miss means elapsed nanoseconds times 48000 exceeds 128 billion, without exclusions. Timing covers control changes, candidate rendering and common original FX. This is synchronous host timing, not device XRUN or complete app qualification. Cold construction and loading are outside the interval; first callbacks and resets are not discarded.

Both modes fail. FTZ/DAZ changes only the diagnostic process and is not a production fix. The eight output diagnostic FNV digests agree across these modes, but that is not full fidelity proof. CPU misses show the observations cannot simply be dismissed as wall scheduling delay. Denormal cost, cold buffer page faults and other effects remain hypotheses requiring separate measurements; observed run variance prevents a single-cause conclusion.

Each gzip is a deterministic compression of the raw trace/log named before `.gz`; decompress with Python gzip or `gzip -dc`. MANIFEST.json binds both compressed and original bytes. Source snapshots and input manifests retain the actual pilot versions, which precede subsequent runner hardening. The normal repeat uses the FTZ diagnostic executable with its diagnostic option disabled; INVOCATION.json records its exact binary and input hashes. No original files are overwritten.

No audio corpus or denied LFS object is included here: these are newly generated timing CSVs, diagnostic logs, source and hashes only.
