# Current full-control native stock link

Fresh isolated copy of full-control-candidate, complete compiler objects rebuilt on the unchanged stock physical map. Authored Vibrato+Saga, Synth One LFO/sync, J6 A REF and two-word mailbox are scoped by the snapshot SOURCE.sha256. Manifest remains unchanged across build. Copy persistent source separately before any rerun; validate SHA manifest whose build paths are generated-input headers. No new guard/companion variant, code/linker change, Git, package or emulator.

| Model | XIP capacity | XIP attempted | XIP free | POOL margin after 8,192 B |
|---|---:|---:|---:|---:|
| Elite | 552672 | 574200 | -21528 | 512 |
| Pro | 552672 | 574188 | -21516 | 512 |

Both final ld subprocesses report XIP overflow and no unresolved-reference diagnostics. Compared with exact prior panel Vibrato native checkpoint, this costs +4,060 B per app. Both maps have .pool 335360 B / capacity 344064 B, .ram_text 2748 B, .c1_text 420 B, .xpool 9216 B and .xdata 7775 B. Native stack address layout is from map-summary.log; no runnable image or stack high-water/runtime bound established by a failed link. No release claim.

Rerun from manifest-matched isolated snapshot with no stale *.o: JIELI_TOOLCHAIN=WORKSPACE/prereqs/toolchain AC79_SDK=WORKSPACE/prereqs/sdk X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_ARP_HOOKS=1 X0X_SYNTHONE_PATCH=1 X0X_S1C=1 PYTHONPATH=SNAPSHOT python WORKSPACE/perf-final-proof/sj6-knob-persistence/native-link-only.py --board smk37 --model MODEL. The wrapper intercepts immediately after native ld and does not run bootloader/package.
