# Part Sound with wet render headroom

Lab checkpoint only, no installer promotion. Build Elite or Pro with `X0X_UI_SMALL_CANVAS=1 X0X_CHICAGO_FOLD_RESIDUAL_MIN=1 DNBOX_LAB_D9_SUFFIX=1 X0X_D9_TANH_BOOT=1 DNBOX_LAB_ARP_EVENTLESS=1 X0X_MIAMI_N513_BOOT=1 MODEL=elite|pro bash build.sh`, and the usual native JieLi toolchain and SDK.

This combines the three-strip Sound snapshots from the previous Part Sound checkpoint, the tested state-preserving render schedule, and the eventless ARP advance. The immutable D9 saturation and Miami filter tables are losslessly decoded before audio starts. The cold and loaded native RAM snapshots on both models reproduce all 1,026 D9 and 513 Miami words exactly. All six qualifying loaded runs adopted 528 private cymbal results with zero fallbacks and kept the ordered mixer. Separate host tests cover full machine state and sample/event parity for the ARP, cymbal and master changes; they do not assert inter-emulator PCM alignment of separate realtime runs.

| MIDI offset | Elite worst | Pro worst | Late callbacks / dropouts |
| ---: | ---: | ---: | ---: |
| 0 ms | 10,574 µs | 10,589 µs | 0 / 0 both |
| 2 ms | 10,581 µs | 10,540 µs | 0 / 0 both |
| 5 ms | 10,266 µs | 10,339 µs | 0 / 0 both |

All six native emulator runs use the pinned 922-event source, 360 MHz strict with no skipped cycles, unchanged loaded NOR and Synth One 518 with arp, delay and Room on and Chicago on B. Worst measured block has 220 µs to the 10,809 µs target. Other MIDI offsets are not implied by this receipt.

Flash: 920 B free Elite, 932 B Pro. Shared pool: 20,680 B after the 8,192 B fixed reserve, both. The canvas removes 26,880 B; exact tables and the private cymbal helper use back 10,816 B, leaving 16,064 B net shared RAM saved versus the original canvas. Reference and scheduler-only branches have different tradeoffs. No PSRAM, no relaxed linker bound, no skipped controls.

Package SHA-256:
```
9138844c5992003d48b27c2a922cd29cf8aadd818faa47f850f479ccb8bf5811  x0x-smk37-elite.fwsc
49592bb5140635204c04bf60a85bdfde28c64c35e27dbced24ac27bf4a1a6886  x0x-smk37.fwsc
```
