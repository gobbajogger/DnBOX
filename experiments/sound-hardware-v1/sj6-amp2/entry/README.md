# Existing SuperJ6 application ingress

The initial compact AMP2 and range-dictionary composition (source tree `18f29e2ce3d83d034a76c6ca6ae79962c12d2350`) fits both stock models, but its four new secondary envelope controls are only reachable by a direct runtime call. This host test takes the application selector, project mirror, live command queue, two actual Juno slots and full stereo render path.

Two selected stock patches on different slots played 32,768 stereo frames per launch. Injecting the four tagged IDs through the existing `engine_set` function changed zero of 65,536 audio words. A supported stock attack control changed 65,396 audio words, starting at frame 64. The sounding baseline had 65,408 nonzero words. Selecting index 4 was refused. These selectors expose four stock patches; there is no complete original bank selection in this composition.

The ordinary command queue packs eight bits of parameter index. Runtime IDs 375–378 consequently reach the consumer as 119–122. The public Juno directory admits indices 0–17 and rejects those four. The project packs the current factory in one byte, and the deferred selection token currently unpacks only seven bits of factory number. Original banks need 640 distinct identities. A correct full importer needs a separate stable identity and raw field path.

Two existing representation-level DSP and runtime gate tests also fail to compile against this alternative, as logged here. They reach into removed voice and damper members and call the removed single-channel render API. Their old behavioural contracts still need equivalent verification if the replacement is proposed for the production branch.

Reproduce from the named compact candidate and a fresh output directory. The stock host generator requires the pinned generated assets described by the repository. The initial reference is separate from the typed event candidate.

```sh
python3 replay.py /path/to/compact-amp2-dictionary /tmp/sj6-original-entry
```
