# Native SuperJ6 build and physical-input replay

Commit `364271ad54fc36b8d6a7dbe0f1bdc13edbe4df52` provides the guarded factory loader. Both boards built from this published source using the pinned JieLi Linux toolchain. The three configurations are ordinary firmware, SuperJ6 alone, and SuperJ6 with the Synth One editor. All six applications, ELFs, loaders and packages reproduced their earlier SHA-256 hashes. The ordinary outputs also match shipping firmware on both models. [Published-source rebuilds](published-builds.json) · [Footprint, image sizes, symbols and focused audio results](verification.json) · [Six native build logs and focused test outputs](focused-and-native-logs.tar.gz)

## Stock comparison on simulated hardware inputs

Each target booted from its own package before receiving the same 922 events as stock at strict 360 MHz. Independent package decoding verified all 643,072 protected flash bytes and the native application. The initialized 405,504-byte persistent region matched calibrated stock. The stock-seeded candidate negative control failed the identity check on both boards. The DAC comparison covers all 333,312 stereo frames per run. The final project comparison covers all 36,420 bytes at the running image’s project symbol.

| Input shift | Elite peak µs | First differing Elite frame | Changed Elite project bytes | Pro peak µs | First differing Pro frame | Changed Pro project bytes |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| +0 ms | 11,238 | 220,421 | 5 | 11,252 | 160,324 | 6 |
| +1 ms | 11,272 | 160,324 | 11 | 11,255 | 199,811 | 4 |
| +2 ms | 11,404 | 199,279 | 5 | 11,259 | 102,402 | 6 |
| +3 ms | 11,255 | 102,402 | 3 | 11,261 | 199,284 | 7 |
| +4 ms | 11,293 | 102,402 | 4 | 11,246 | 201,349 | 4 |
| +5 ms | 11,271 | 155,712 | 5 | 11,092 | 155,712 | 3 |

Every scene completed 648 audio halves. Each had one startup late transfer and xrun, and zero additional late transfers during live input. Peak margin against 11,609 µs was 205 µs on Elite and 348 µs on Pro. Both fall below the requested 700 µs. Each board reproduced the stock warm boot and all 30,208 stereo frames and project bytes in the saved Clip startup. This includes the ten nonzero channel words on DAC edge 4. [Per-shot firmware identity, startup, audio, project and timing measurements](physical.json)

The combined build with the Synth One editor also ran at +1 ms on each model. Elite reached 11,323 µs and first differed at frame 120,896, with ten project bytes changed. Pro reached 11,316 µs and first differed at frame 199,282, with two project bytes changed. Both finished 648 halves with one startup late and zero additional late transfers. The other five offsets were measured on the SuperJ6-only build in the table. [Combined-profile +1 ms comparisons](combined-plus1.json)

## Reproduction

From a checkout of the source commit above, export `JIELI_TOOLCHAIN` and `AC79_SDK` as in the repository build instructions. Build each board sequentially. Record or copy all four native outputs before switching configurations. [Native build instructions](../../../../BUILDING.md)

```sh
for features in "0 0" "1 0" "1 1"
do
    set -- $features
    for model in elite pro
    do
        BOARD=smk37 MODEL="$model" X0X_SUPERJ6_FACTORY="$1" \
            X0X_SYNTHONE_PATCH="$2" sh build.sh || exit 1
        board=smk37
        if [ "$model" = elite ]; then board=smk37-elite; fi
        sha256sum "build/$board/x0x.bin" "build/$board/x0x.elf" \
            "build/$board/loader/ota.bin" "build/x0x-$board.fwsc" || exit 1
    done
done
```

Use a new output directory per board/configuration and the matched ELF and package with the [strict replay and scoring commands](../native/README.md#reproduce-the-native-experiment). A missed timing target gives a failing exit status after the sweep is written. Inspect all twelve records. The comparator authenticates the candidate’s protected region before comparing the warmed persistent region against the model’s stock seed.
