# Synth One INIT on bass A and B

Select Synth One on either bass part. K1–K4 on FM-1 and K5–K8 on SMK-37 address the four columns on the selected row. The same parameters are available in the part list. The SOUND page selects the engine and shows the active preset. INIT is the one Synth One preset admitted by this version of Sound. SuperJ6 keeps its existing pages. [FM-1 and SMK caption comparison](board-captions.md)

| Row | Column | Factory ID | Display label | Physical range | One position | Display |
|---|---:|---:|---|---|---|---|
| Oscillator | 1 | 2 | OSC BALANCE | 0–100% | 100/127 percentage points | Whole % |
| Oscillator | 2 | 5 | OSC 1 LEVEL | 0–100% | 100/127 percentage points | Whole % |
| Filter | 1 | 16 | CUTOFF | 64–22,050 Hz | 21,986/127 Hz | Hz or kHz |
| Filter | 2 | 17 | RESONANCE | 0–98% | 98/127 percentage points | Whole % |
| Filter | 3 | 19 | FILTER ENV | 0–120% | 120/127 percentage points | Whole % |
| Filter ADSR | 1 | 22 | FILTER ATTACK | 0.5–2,000 ms | 1,999.5/127 ms | ms or s |
| Filter ADSR | 2 | 23 | FILTER DECAY | 5–2,000 ms | 1,995/127 ms | ms or s |
| Filter ADSR | 3 | 24 | FILTER SUSTAIN | 0–100% | 100/127 percentage points | Whole % |
| Filter ADSR | 4 | 25 | FILTER RELEASE | 0–2,000 ms | 2,000/127 ms | ms or s |
| Amp ADSR | 1 | 26 | AMP ATTACK | 0.5–2,000 ms | 1,999.5/127 ms | ms or s |
| Amp ADSR | 2 | 27 | AMP DECAY | 5–2,000 ms | 1,995/127 ms | ms or s |
| Amp ADSR | 3 | 28 | AMP SUSTAIN | 0–100% | 100/127 percentage points | Whole % |
| Amp ADSR | 4 | 29 | AMP RELEASE | 4–2,000 ms | 1,996/127 ms | ms or s |

The descriptors use 128 evenly spaced edit positions including both endpoints. Untouched INIT values retain their original precision. The display reads the physical value applied by Sound. Frequencies below 999.5 Hz and times below 999.5 ms round to whole Hz or ms. Larger values use kHz or seconds, with one decimal below 10 and whole numbers above that range. The percentage rows round to whole percentages. [Factory descriptors and position mapping](https://github.com/gobbajogger/DnBOX/blob/80b7e1db59e5c3eceb2b3530e80c2cedf29c5b7f/firmware/src/app/s1_dry_part.c)

An edit carries the part, factory ID and current patch revision. Sound applies it at the next render boundary. Until then the panel shows the last applied value, and a save attempt returns busy until the edit is applied. If another encoder requests a change on that part in the same block, the panel shows `EDIT BUSY - RETRY`. The other bass part has its own edit slot. A preset change or project load invalidates requests for the former revision. The sound cell stores the edited positions and mask for each part. Project reload restores both cells. [Panel regression](../../../../../tests/host/ui_synthone_patch_test.c)

## Reproduce

Use a local repository containing the FWUI candidate, integration base and pinned Sound commit. For stock links, supply the JieLi toolchain and SDK used in the [stock receipt](../editor-stock/stock-fit.json). Run this from the repository root with a fresh output directory.

```sh
python3 tests/host/qualify_fwui_s1.py /tmp/fwui-synthone --repo . --fwui 61827a714ca7b240c01a55ed347b5fd7b847c7c2 \
  --test-ref HEAD --boards --native-receipt docs/lanes/fwui/current/synthone-editor/native-replay.json
```

The qualifier combines tracked source inputs, verifies the original-synth inputs, compiles the host panel and provider, and links the patch-off and patch-on configurations for both stock models. [Host and visual qualification](qualification.json) · [88 framebuffer captures](frames.tar.gz) · [Frame hashes](frames.json) · [Four native links](native.json)
