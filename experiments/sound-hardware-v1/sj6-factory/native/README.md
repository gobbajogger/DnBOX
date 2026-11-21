# Compact SuperJ6 on the current Sound firmware

The linked experiment combines the decoded waveform/PWM, AMP2 and master-volume entry points with the corrected two-layer oscillator. It applies to the same firmware used for the current stock measurements. The ELF retains both typed entry points using linker roots. The normal shipping preset menu has four Juno choices. The 122,359-byte full factory table and a selector for the 640 programs are outside this image.

| Native measurement | Stock Elite | Compact Elite | Stock Pro | Compact Pro |
| --- | ---: | ---: | ---: | ---: |
| Occupied application, bytes | 546,516 | 550,436 | 546,508 | 550,424 |
| Free XIP, bytes | 6,156 | 2,236 | 6,164 | 2,248 |
| Free pool, bytes | 8,972 | 8,972 | 8,972 | 8,972 |
| Pool above 8,192-byte floor | 780 | 780 | 780 | 780 |
| Packaged firmware, bytes | 651,732 | 651,732 | 651,730 | 651,730 |

Both compact targets use the standard board model and linker layout, size optimization on the two Juno DSP units, disabled diagnostic counters and the ordinary stock BREAK table. The calibrated eight-second scene exercises the panel, encoders, MIDI, sequencer, DAC and saved project. The separate published-Juno preset test measures what a user hears when selecting each of those four voices. [Host test and per-preset measurements](../native-host/README.md)

**Correction.** The first twelve replay shots restored the complete stock flash over the supplied compact package. They ran stock code. The previous compact physical parity and timing claims are withdrawn. The 17 fixed-input host scenarios independently matched all 50 stock audio/state files. Native image and pool measurements above came from the compact link.

The repeated six offsets per board used authenticated compact firmware with stock-equivalent user state. All twelve DAC recordings and final projects differed from their same-offset stock references. The first differing stereo frame ranged from 102,402 to 288,833. Final projects differed in one to eleven bytes. The worst peaks were 11,396 µs Elite and 11,288 µs Pro. Each run completed 648 halves with one startup late transfer and zero additional scene late transfers. [Package checks and all twelve measurements](physical.json)

## Reproduce the native experiment

From the Sound checkout, choose a new work directory and export the project's AC79 SDK and JieLi toolchain paths. The firmware base is `dea023ce632538621b59bb307ebcaf440805b032`. Applying the patch with the index produces tree `bfd39793b1bf792a0b43b6d635ce3318058717e1` on that exact parent. Build the two models sequentially in each tree because the generator directory is shared.

```sh
bench="$PWD/experiments/sound-hardware-v1/sj6-factory"
work=$(mktemp -d)
git worktree add --detach "$work/stock" dea023ce632538621b59bb307ebcaf440805b032
git worktree add --detach "$work/compact" dea023ce632538621b59bb307ebcaf440805b032
gzip -dc "$bench/native/current-head.patch.gz" | git -C "$work/compact" apply --index -
test "$(git -C "$work/compact" write-tree)" = bfd39793b1bf792a0b43b6d635ce3318058717e1
for tree in stock compact
do
    for model in elite pro
    do
        (cd "$work/$tree" && BOARD=smk37 MODEL="$model" sh build.sh) || exit 1
    done
done
python3 "$bench/replay_pwm_defaults.py" "$work/compact"
```

The oscillator runner accepts this prepared compact tree directly. Use its controls, range, holdout and PWM String Pad suites as documented in the [oscillator comparison](../oscillator/README.md). Both native loaders matched the stock loader for their model. [Build identities, symbols and host checks](results.json)

The physical replay uses the repository's strict driver and matched board binaries. The calibrated source of `fm1-live` is `ba83317aa55cbcdb2830c5da97d5f64b763499e2`, built here with Rust 1.85.1. The stock +1 control on each board reproduced the archived image, seed, project and DAC digests and all timing and late counts. The command below repeats the six shifts on each candidate and compares each capture against the two archived stock measurement packets. The replay writes `sweep.json` only for a completed run. It returns a failing status when any completed shot misses its timing target. The example records all board/shift failures before returning that status. Use a fresh output directory for each replay. The optional raw-reference flags below authenticate the stock ELF, DAC and project before reporting the first zero-based differing stereo frame and the number of changed frames and project bytes. All writes are to the emulator's disposable flash file.

```sh
failed=0
for model in elite pro
do
    board=smk37
    if [ "$model" = elite ]; then board=smk37-elite; fi
    for tree in stock compact
    do
        build="$work/$tree/build"
        out="$work/$tree-$model-sweep"
        python3 "$bench/../strict-timing/replay.py" --emulator "$FM1_LIVE" \
            --elf "$build/$board/x0x.elf" --fwsc "$build/x0x-$board.fwsc" \
            --output "$out" || failed=1
        test -s "$out/sweep.json" || exit 2
        for shift in 0 1 2 3 4 5
        do
            python3 "$bench/native/score_scene.py" "$model" "$build/$board/x0x.elf" \
                "$out/shift$shift" "$shift" --stock-seed "$work/stock-$model-sweep/seed/flash.bin" \
                --stock-folder "$work/stock-$model-sweep/shift$shift" \
                --stock-elf "$work/stock/build/$board/x0x.elf" || failed=1
        done
    done
done
exit "$failed"
```
