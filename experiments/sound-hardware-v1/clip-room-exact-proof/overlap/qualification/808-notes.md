# 808 front stage on joined six-voice B, both-kit overlap arm

Baseline same recipe as overlap lazy on e00f689d with core plan 3, partial peer Synth One and existing exact tanh shortcut. Native timing is the project's 5.3 s Pro workload, fresh virtual NOR with exact controller/project, six MIDI strikes on channel 3. No hardware, packaging change or deployment.

The current Core1 PC trace mapped cy_run 7,175,068 ticks; other 808 voice_run 7,119,324; bank_block 3,670,342; 808 lane mix 1,533,022; d8_render 1,318,977. It motivated porting the two-pass narrow cymbal front to the newer exact low-poly/overlap composition. The change also reuses four exactly symmetric BQ3 numerator products per sample and reconstructs the original six BQ3 history values on early retirement.

Host exact: all four buses/full state in 8,205,582 full-kit frames, 64k spans; isolated cy_run exact in 3,152,732 random samples, 256k calls and all 496 forced actual rewind offsets. Native raw stream hash differs; timing and repeated DMA spans mean this stream is not a direct equality check.

| Pro | Stale spans | Parity late | Peak µs | Last completed µs | XIP bytes free | Pool bytes beyond reserve |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Lazy baseline | 49 | 50 | 16047 | 13906 | 4980 | 27392 |
| Cymbal front stage | 49 | 50 | 16081 | 14610 | 5012 | 27392 |

Both retained 340 completed halves, six gated Synth One voices on B with A stock, worker mask 3 and zero giveups. The candidate adds stack handling (native cy_run locals 340→580 B, plus front 20 B and saves). Reject on current code, no native timing improvement.

Follow-up: the baseline PC at the variable shift in bank_block drew 418,467 Core1 attribution ticks. The phase bank currently sets pending only at initialization (osc 4 and 5), not on per-note tuning. Removing recurring pending byte read plus variable bit shift after the first sample does not alter RNG order, phase, or tails. The tiny hoisted-mask patch explicitly preserves zero-length calls and high unused bits. Targeted bank proof: 1,051,104 frames over 65,536 spans, 98,718 forced simultaneous natural+pending event setups, clean output, cowbell, and state comparison. Full 808 C: 8,205,582 frames, four buses and state bit-exact. Native test separate.
Native pending hoist rejected: same 49 stale spans / 50 parity-late, peak 16,161 vs 16,047 us and last 14,669 vs 13,906 us.
