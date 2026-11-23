# CPU samples from the slowest factory callbacks

The eight programs with the most thread-CPU deadline misses in the archived original 694-program run were replayed eight times each. Ties use the largest thread-CPU time, then the program index. The sampling timer counts CPU time and records the interrupted instruction only inside the note/control/render call. The original IEEE settings were retained. All 64 complete render checksums matched their archived checksums and every output word was finite.

| Function | Samples of 7,416 | Share |
| --- | ---: | ---: |
| Original ladder filter | 1,426 | 19.2% |
| Unresolved instructions in libm | 894 | 12.1% |
| Compact waveform word lookup | 823 | 11.1% |
| Compact note renderer, excluding called functions | 565 | 7.6% |
| Oscillator morph calculation, excluding called functions | 517 | 7.0% |
| Compact block reader | 458 | 6.2% |
| Four-word wavetable lookup | 370 | 5.0% |

The timer interrupted this run at approximately one millisecond of process CPU. These are instruction-location counts from a Linux host. The sampler only counted the work done inside callbacks. Its own overhead disqualifies this run for deadline acceptance. The original uninstrumented timing report remains in the [callback packet](../stock-dry/factory-callback-694.tar.gz).

```sh
python3 profile.py "$FACTORY_BUILD" ../stock-dry/factory-callback-694.tar.gz /tmp/synthone-profile
```

The command is for Linux x86-64. It verifies the frozen callback source, selects the programs from the archived CPU measurements, rebuilds the harness with the original compiler flags, checks each render checksum and resolves sampled instruction addresses against the actual binaries. [All function counts](callback-pcs.json)
