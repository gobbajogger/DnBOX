# Native adaptive wet handoff, identical-code A/B

Copied enabled guarded ELF, and changed only the initialized volatile experimental handoff flag (one file byte, same offset in both binaries). Verified exact identity of all other bytes, symbol map, every executable/data-asset section, both note and 922-event owner input, same paired decoder and loaded project recipe. Only user-authorized disposable virtual NOR; zero erases/programs and backed bytes unchanged. Compiler build defaults and media do not differ between these two arms.

| Board | No handoff worst / XRUNs | Handoff worst / XRUNs | Grants / tiles |
|---|---:|---:|---:|
| Elite | 15239 µs / 174 | 16173 µs / 88 | 968 / 9793 |
| Pro | 15565 µs / 191 | 14870 µs / 76 | 1053 / 10317 |

All four reached the actual S1 voice kernel, 319488 PCM frames, and five checked parameter owner replies. Disabled arms had zero grants and remote tiles. No transport faults or skipped simulator cycles in any arm. Core 1 stack paint high-water 2512/4352 B in all four. These runs establish a meaningful reduction in missed blocks due to allowing the handoff on this tape, not deterministic equality of application-time MIDI events or production approval. Worst block worsened on Elite, improved on Pro; both criteria still fail strict ≤10809 µs with zero late/XRUN. Do not enable by default. Real worker fault recovery, multiple active owner/plugin states, timing margin/offset sweep and product assets are unqualified.

ELFs, off/on:
- Elite 433f067639bf0fd93114b8932c030d86ab025f8cef6220cb759fcb321802ba5f / 761ba497a2da0dd231dd3249f398f25abdd8197768f65f350c83d8e4f65f5ef6.
- Pro 1f3d2fbb1984dd7466f59365d099959eec82c22d4ee77fbd4a613483c21439f9 / 9e8e35903926bdae32076022ac485b902fffa68be1936afb1af22be521acb445.

Separate SuperJ6 evidence: original current packed loader uses exact signed16/32768 conversion (all 65536 encodings and authentic zone slices verified), no gain opcode in the loaded Sub SFZ zones, and the same native descriptor reaches the original Core sample buffer path on the traced ingress. No evidence of hidden normalization in that loader; full app resource selection remains unobserved. Do not retune individual patch or assign arbitrary role gain.
