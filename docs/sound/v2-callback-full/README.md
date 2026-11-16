# Complete 694-program host callback characterization: FAIL

Source: `521963ede8261c87a4f553d68664ece8808bed0e`. Normal floating mode only. All 694 original SynthOne programs, 520,500 callbacks and 66,624,000 frames completed on each host. Every block is 128 frames at 48 kHz; no warm-up or outlier exclusions.

| Host | Wall misses | Thread CPU misses | Nonfinite words | Benchmark tracer |
|---|---:|---:|---:|---:|
| Local shared workspace | 10,159 | 9,771 | 0 | 0 |
| Separate managed workspace | 70 | 62 | 0 | 0 |

Both fail. The large host-to-host variance does not establish a cause or excuse either failure. The local shared workspace was not isolated from other agents or read-only diagnostic work; no exclusive-CPU or device-real-time claim is made. Actual benchmark process tracer is observed, not inferred from Python.

Both compiled executable hashes are `aa2408d6882a1bc2751785c4968ef376dbd5de65466f4ba45636cb0bb86cfc00`. Both bound input manifests hash to `eb06fa673ef0e35dc79d597ed526a456285102282839b8dd0d6a7c06aedb5c40`. Both command streams hash to `460d0869c7287ab10dc2ac1928e56b899e8dc089df021ab82d8704ab28cfe7af`. The recovered managed result/environment bytes were rehashed against the managed report, not reconstructed from approximate counts.

These are synchronous host control/render/common-FX timing measurements with fixed fixture tuning, not device XRUN counters, independently qualified original FX, all-engine native fit or original iOS app equivalence. All earlier failing pilots remain valid recorded failures. Complete corpus finite output does not make S1 PASS because the zero-miss condition failed.

## Recovery and publication scope

Small results, environment, common input manifest, local command manifest and local run log are retained here. Full raw traces and per-program output remain preserved in both execution workspaces and are identified by hashes in the receipts. Local raw trace SHA256 is `6f3dee176a65bf2ce43333d232b284dae239c8c8194911d0a705527b6a159501`; managed trace is `3829cffacb7099534985d036f36846673f6c7b0a4b44ad9e3b156f2ee7739ee1`. The independent managed task retains its complete output manifest. Full new trace publication is not claimed: a separate small pilot trace plugin write is under unresolved automated-review denial, documented in v2-callback-current-status.md. No alternate upload, encoding or LFS retry was used.

Next: independently recalculate per-program/event miss distribution on the separate host, diagnose a fidelity-preserving correction, and retain all before/after observations. Do not shorten effects, cut voices or adopt a different floating mode merely to turn the timing check green.
