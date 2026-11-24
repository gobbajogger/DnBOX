# Build-9 chorus evidence

The Build-9 conductor pass defines these fields in order. The defaults below are binary32 words unless indicated otherwise.

| Position | ID | Field | Default |
|---:|---:|---|---|
| 44 | 40 | neonchorusIEnable | 00000000 |
| 45 | 41 | neonchorusIIEnable | 00000000 |
| 46 | 42 | neonchorusRate | 3eced917 |
| 47 | 43 | neonchorusDepth | 3e4ed917 |
| 48 | 44 | neonchorusMix | 3f0147ae |
| 49 | 208 | neonchorusVersion | Inverse of 1 over [0,1] with taper 1, platform word unmeasured |

[Build-9 conductor control contract at 5d18b182](https://github.com/gobbajogger/DnBOX/blob/5d18b182955d107c7f8ca09fb5b0b08a83a6adc9/experiments/superj6-conductor-source-20261115/control_contract.json)

All 640 programs in the authenticated export carry both switch words as positive zero or one. OFF occurs 300 times, I 200, II 97 and I+II 43. Rate, depth and mix are present together in 26 programs. Version is present in 68 programs (46 zero, 22 one). Of the 26 parameter triplets, 22 specify version one, three specify zero and one has no version field. The authored rate words are 3eb63af3 and 3eced917, the depth words are 3e4ed917 and 3e553ed0, and mix has the single word 3f0147ae. The export SHA-256 is ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8.

The pinned local app bench records the original mode byte as 0 OFF, 1 I, 2 II and 3 I+II after the app setter. The same route selects the legacy TempoChorus/AKChorus for version values other than one and J60V2 for version one. The J60V2 object passes its rate scale, depth scale and mix to the kernel at addresses 1, 2 and 3, with mode at address 0 and ramp at 4. The legacy reconstruction in this bench is approximate. The evidence does not establish the conversion from authored dial words to the two physical rate/depth laws, the original dry/wet gain curve, or the scope of the global tempo switch.

The Build-9 AU digest in the earlier component replay is a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1. At 48 kHz its three active 4,096-frame component receipts have distinct SHA-256 values (e653e0b4, 7a942b0d, 761a38c6). The started mode-I zero-mix clip and unstarted mode-I full-mix clip both hash to 91c0d1714393d7bae29b14b259af99290c99383d620846a10bc569bd4fcf50f7. These observations establish structural distinctions and zero-mix bypass. They do not establish the mode sweep shapes, base rates, delay interpolation or history transitions.

Local evidence paths are sj6-private-final/bench-work/sj6-fullbench/bench/fx.py, j60.py and j60check.py, perf-final-forecast/experiments/sound-hardware-v1/sj6-factory/original/component-replay.json and the earlier perf-final-proof/sj6-chorus-bypass packet. Those source inputs were read in place. This checkpoint introduces no original executable, preset data or PCM into the patch.
