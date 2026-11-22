# Juno render cost and signal preservation

The PI32v2 test starts a fresh Juno with the shipping chorus, holds 0, 1, 2 or 6 notes and renders ten 128-frame blocks at 44.1 kHz. Notes start at MIDI 32 in seven-semitone intervals with velocity 99. The instruction count covers the whole fixture, including initialization and its final stereo checksum. At six voices it fell from 1,273,618 to 1,071,316. Each voice count produced the same checksum in the two revisions. [All eight counts and checksums](results.json)

Run from the repository root with both commits in local Git history, the Linux JieLi 20260730.1 toolchain and fm1-emu from emulator source `ba83317aa55cbcdb2830c5da97d5f64b763499e2`.

```sh
python3 experiments/sound-hardware-v1/juno-render/replay.py "$JIELI_TOOLCHAIN" "$FM1_EMU"
```

The separate host fixture varies the block length from 0 to 257 and changes control values, retriggers notes, releases keys and changes the chorus mix. It writes both output channels and the complete synth and chorus state after each of 4,000 blocks at each sample rate (22,050, 44,100 and 88,200 Hz). GCC 14 on x86-64 produced the following identical SHA-256 digests for the baseline and candidate. The host structure bytes include implementation-specific padding. Compare both revisions with the same compiler and ABI.

| Initial notes | Stereo and state SHA-256 |
|---:|---|
| 1 | `9db7e04aead0cd888ce06d4e45e1d372d9c892e3489e74aca22589a609ce1e44` |
| 2 | `b010c387626ece60db3db760ddbde717b0f09b5e998b40791c53ea9ed7f1007b` |
| 6 | `e7ffd5edc65ff98218d3104746c3478f3f7cffe0517f0c970fab82cf4701eec7` |

```sh
work=$(mktemp -d)
for ref in 6c91d26ecb621b023334900c09e3705d2a91d86e 49662e4d437421e686a6cc04b806cd0d700bab74; do
    git show "$ref:firmware/src/dsp/juno6.c" > "$work/juno6.c"
    cc -O2 -std=c99 -DX0X_HOST -ffp-contract=off -Ifirmware/src/dsp \
        "$work/juno6.c" experiments/sound-hardware-v1/juno-render/host_render.c -o "$work/render"
    for voices in 1 2 6; do "$work/render" "$voices" | sha256sum; done
done
```

The populated Elite and Pro deadlines use the complete firmware and its actual DMA counters in the separate fm1-live heavy-scene test.
