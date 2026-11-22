# Dual Juno renderer experiment

Ordinary Juno selection uses the shipping six-voice allocator, envelopes, oscillator, filter and shared chorus. A decoded per-layer waveform switches an idle owner to the two-layer engine and selects the factory oscillator for that layer. An idle decoded AMP2 edit also selects the two-layer engine, using the existing BLEP oscillator until a waveform is supplied. A held or releasing shipping voice rejects AMP2 without changing voice or runtime status. A sounding two-layer owner accepts AMP2. Tagged master gain applies after the chorus in either renderer, including its tail. Cold preset adoption restores the shipping renderer.

| Model | Application bytes | Free XIP | Free pool | Pool above reserve |
| --- | ---: | ---: | ---: | ---: |
| Elite | 551,492 | 1,180 | 8,972 | 780 |
| Pro | 551,480 | 1,192 | 8,972 | 780 |

Both builds use the stock flash layout. The early dual Elite build with the shipping FLOAT32 BREAK table had 236 bytes of free XIP. The final dual build uses an exact packed BREAK lookup. Comparing the packed lookup with the table at the shipping source SHA gave identical words on all six lengths, all 256 byte-valued pot settings and UINT32_MAX. The original generator reproduces all 300 packed words under four host optimization levels. The PI32v2 emulator reproduced the 1,542 expected words. [Native identities](results.json) · [BREAK measurements and integer bounds](break-results.json)

All four published shipping Juno programs match the shipping head on 262,144 stereo Float32 words. The 48 synthetic typed settings span the selected layer, all four waveform stops, both PWM gates and three PWM amounts. All 147,456 stereo Float32 words match the compact renderer under a two-note progression. Its SHA256 is `cd5815d27df86142b0c8e167f3971e6498d293d727870d9c32924394ce1df6d1`. The shipping test uses C4 at velocity 100 with release at frame 12,288 and 32,768 frames per program. The direct tests also check voice age, held and released ownership, master, the first AMP2 edit, subsequent live edits, cold reselection and the original voice-stealing/sustain/bend behavior.

The firmware uses the stereo renderer. In this measured experiment a direct call to the original low-level mono helper on an active typed owner returns silent stock voices. The public runtime and hardware stereo comparisons above exercise the stereo path. This fixture does not select the 640 factory programs from the panel or measure complete original AU parity.

## Physical input with package-owned flash

Each board's replay initialized fresh flash from its own package. All 643,072 protected bytes matched that package, and all 405,504 persistent bytes matched initialized stock before input. Cold and warm startup reproduced stock audio and project bytes on both boards. The +1 ms scene first differed at stereo frame 120,896 on Elite and 199,811 on Pro. Elite changed one project byte and had an 11,301 µs peak. Pro changed 11 project bytes and peaked at 11,337 µs. Each completed 648 halves, with one raw startup late transfer and zero additional scene late transfers. [Native and stock identities, same-offset differences and exact frames](physical.json)

## Reproduction

Set the standard JieLi toolchain and AC79 SDK environment variables first. The second patch applies to the measured compact tree. Run builds sequentially because both models use its generated-header directory.

```sh
bench="$PWD/experiments/sound-hardware-v1/sj6-factory"
work=$(mktemp -d)
for name in stock compact dual
do
    git worktree add --detach "$work/$name" dea023ce632538621b59bb307ebcaf440805b032 || exit 1
done
for name in compact dual
do
    gzip -dc "$bench/native/current-head.patch.gz" | git -C "$work/$name" apply --index - || exit 1
done
gzip -dc "$bench/native-dual/from-compact.patch.gz" | git -C "$work/dual" apply --index -
test "$(git -C "$work/dual" write-tree)" = 27823de5e1f97fc8ac21e7eff62d6bc0014135b1
for model in elite pro
do
    (cd "$work/dual" && BOARD=smk37 MODEL="$model" sh build.sh) || exit 1
done
sh "$bench/native-dual/compare.sh" "$work/stock" "$work/compact" "$work/dual" "$work/host"
python3 "$bench/replay_pwm_defaults.py" "$work/dual"
```
