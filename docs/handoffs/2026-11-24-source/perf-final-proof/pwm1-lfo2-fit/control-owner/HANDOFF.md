# Shared Synth One/SuperJ6 foreground owner admission

Actual normal production links, with identical locked model configuration and exact PWM2 + page-matrix source:

| Variant | Elite over app segment | Pro over app segment | Net reduction each |
|---|---:|---:|---:|
| Baseline | 12,184 B | 12,176 B | |
| Share checked owner-index leaf | 12,136 B | 12,128 B | 48 B |
| Share checked runtime-pointer leaf | **12,120 B** | **12,112 B** | **64 B** |

The shared contract is exactly kind equality and status == OK under the already-held existing foreground CQ lock. The pointer variant emits a 32-byte leaf; representative compiled reductions include Synth One snapshot 172→156 B and SuperJ6 AMP1 read 136→122 B. The callers still validate input before acquiring their original lock, still guard stale/busy edits and particular factory/preset/wave state, and still write only the owner-specific mailboxes. No audio-boundary, active math, original-runtime DSP payload, layout, or file format changes. RAM-text, core1-text, XDATA, and POOL (+512 B above 8,192 reserve) stay unchanged. The helper is opt-in by local source patch, not merged. This is not material against the remaining stock-region gap and should not hold up active LFO sync work; if later used, remeasure the current joined tip and qualify native timing. New helper consumes no LFO interface or storage.

Do not combine the mutually exclusive patches. Reproduce by taking exact PWM2 + compact-page candidate as documented in sibling page-matrix, applying control-owner-pointer.patch or the index arm, and running the unchanged previous native-link-only harness; it stops before package generation. Differential checks run using sibling host-with-existing-inputs.sh with both engine features and compact S1 enabled; standalone fixture adds -Itests/host. The PWM2 receiver fixture uses the existing PWM2 test-only patch. Stock paired regression results and full four-bank 49,152-frame PCM SHA are separately recorded, no PCM blob retained. Two broader legacy fixture failures are identical in baseline and pointer under this configuration and remain failures, not a passing complete gate. No emulator, installers or publication.
