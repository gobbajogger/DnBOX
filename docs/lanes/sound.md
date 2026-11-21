# Sound lane

## Timing decision

The note-cost planner with audio-owned startup reached **10,409 µs on Elite** and **10,377 µs on Pro** across +1 through +5 ms. All ten shifted runs completed 648 audio halves in 8 seconds with zero raw or scene late. That leaves 1,200 and 1,232 µs of headroom against 11,609 µs. Both models retained stock application and package sizes, byte-identical loaders and 8,972 B of free pool. **Behavior qualification fails.** Each same-offset physical project differed from stock. Pro +1 differed in one byte, bass B length 32 versus 35. The production firmware remains at its previously qualified source. S6 is still FAIL. [Timing, flash, project and DAC records](../../experiments/sound-hardware-v1/strict-timing/note-cost-and-owned-boot.tar.gz)

A nonzero startup fixture exposed a second failure. Stock's first generated block reached the DAC from bank 1 on edge 4. The audio-owned startup delivered it from bank 0 on edge 5, one physical half later. Its zero counter describes a different delivery deadline. [Native bank writes, deadlines and physical DAC edges](../../experiments/sound-hardware-v1/strict-timing/first-nonzero-bank-comparison.tar.gz)

## emu360 timing margin

The owner requested a worst block of 10,909 µs or less against the 11,609 µs limit, with zero late and unchanged audio behavior, flash use and 8,192 B pool reserve. Both boards run the stock firmware from `ca0c00a4e5863132de4a66f5d1c477c14d699be5` in `fm1-live` at 360 MHz with `FM1_STRICT=1`. The 8-second scene has 922 events. Each shift moves every press, release and MIDI event by the same amount.

| Scene shift | Current Elite (µs) | Current Pro (µs) | Historical Elite replay (µs) |
| --- | ---: | ---: | ---: |
| +0 ms | 11,402 | 11,289 | 11,406 |
| +1 ms | 11,323 | 11,318 | 11,440 |
| +2 ms | 11,319 | 11,087 | 10,921 |
| +3 ms | 11,282 | 11,241 | 11,438 |
| +4 ms | 11,297 | 11,303 | 11,438 |
| +5 ms | 11,272 | 11,272 | 10,922 |

The worst current shifted run was **11,323 µs Elite** (+1 ms, 414 µs over target) and **11,318 µs Pro** (+1 ms, 409 µs over). Each stock or historical shot completed 648 halves with **0 late during the scene and 1 raw late**. The first IRQ reaches the second DMA half with less than 1 ms left before the next boundary. The separate same-image 2-second warmed boot also records one late. The historical d4088e60 Elite replay reproduced the owner’s 11,440 µs at +1 ms and the previously published 11,406 µs unshifted.

The current Elite unshifted maximum occurred in half 603, at 7.466 s. The 909 and Chicago bass consumed 78,993 and 75,964 guest timer ticks in that half. A diagnostic mixer/planner variant reached 11,291 µs unshifted. It changed the app footprint by −48 B and the first DAC difference occurred at stereo frame 160,324. Its timing does not qualify it for production. The production Elite and Pro images remain 546,516 and 546,508 B. Each retains 8,972 B of pool, or 780 B above reserve. The scene requests BREAK events and the stock image has no optional factory loop bank installed. [Reproduction and measurements](../../experiments/sound-hardware-v1/strict-timing/README.md)

### Native changes under test

| Elite experiment | Unshifted peak (µs) | Occupied application delta | Raw late | Scene late |
| --- | ---: | ---: | ---: | ---: |
| Stock | 11,402 | 0 B | 1 | 0 |
| Planner and settled mixer gain | 11,291 | −48 B | 1 | 0 |
| Mixer and quiet-tail accounting | 11,125 | −240 B | 1 | 0 |
| Planner hysteresis | 11,382 | 0 B | 1 | 0 |
| Chicago finite-check simplification | 11,272 | −96 B | 1 | 0 |
| 909 stereo accumulation, IMA and tom changes | 11,108 | −80 B | 1 | 0 |
| USB initialized before audio DMA | 11,363 | 0 B | 0 | 0 |
| Combined Chicago, 909, mixer and USB ordering | 11,238 | −400 B | 0 | 0 |

The −400 B combination also measured 11,238 µs unshifted on Pro. Initializing USB before DMA lets the first DMA interrupt run on time. Its 8-second captures complete 647 halves because DMA starts later. The stock image completes 648. The scene and stop time stay fixed. A full-size variant specializes the DJ filter, limiter knee and kick path. Fresh native links confirm exactly **546,516 B Elite / 546,508 B Pro** with 8,972 B free pool on both. Each variant has its own ELF, package and factory flash. [Native sizes and differential](../../experiments/sound-hardware-v1/strict-timing/occupied-parity-proof.tar.gz)

The −400 B combination matched all 50 deterministic audio and state outputs across 17 host scenarios. In the unshifted Elite live run, however, five of 36,420 project bytes differed from stock. Two 909 step masks, the 808 pattern length and two sound settings changed. Of the post-PLAY event rows, 296 matched at the same rendered frame, three were present only on stock and one only on the candidate. The phase-sensitive physical setup also yields different project bytes across shifts on unmodified stock. None of these candidates has been enabled. [Control, project, onset and timing receipts](../../experiments/sound-hardware-v1/strict-timing/current-sweep-proof.tar.gz)

### Exact-size emu360 results

Both candidates occupy 546,516 B on Elite and 546,508 B on Pro. Both retain 8,972 B of free pool. The second candidate computes the original 909 nonlinear table coordinate directly from the triangle phase. All 75,497,473 reachable rounded inputs matched word for word. Both candidates passed 17 fixed-input production host scenarios and matched all 50 state and audio files.

| Shift | First exact-size Elite (µs) | First exact-size Pro (µs) | Direct-coordinate Elite (µs) | Direct-coordinate Pro (µs) |
| --- | ---: | ---: | ---: | ---: |
| Control | 11,144 | 11,158 | 10,652 | 10,903 |
| +1 ms | 11,080 | 10,844 | 10,908 | 10,902 |
| +2 ms | 11,148 | 11,142 | 10,877 | 10,813 |
| +3 ms | 11,136 | 11,057 | 10,589 | 10,939 |
| +4 ms | 11,133 | 11,181 | 10,937 | 10,904 |
| +5 ms | 10,931 | 10,827 | 10,834 | 10,582 |

All listed candidate shots ended at 8,000 ms with 922 uniformly shifted events, zero skipped emulator cycles and zero raw or scene late. They completed 647 halves because USB settles before audio DMA starts. Stock completed 648. Extending the +1 ms Pro control on the first candidate to 8,012 ms produced 648 halves and the same 10,844 µs maximum. [Per-shot image, project and DAC identities](../../experiments/sound-hardware-v1/strict-timing/candidate-measurements.json)

For the measured +1 through +5 ms direct-coordinate shots, Elite recorded 10,937 µs at +4 ms, 4/5 measured offsets at or below target and Pro recorded 10,939 µs at +3 ms, 4/5 measured offsets at or below target. At +1 ms the same-offset native project differed from stock in 11 bytes on Elite and four on Pro. Both contain a changed 909 hit mask. The candidate remains experimental. [DSP source, native sizes, fixture and differential](../../experiments/sound-hardware-v1/strict-timing/direct-coordinate-proof.tar.gz)

### Chicago zero-XMod and startup tests

The Chicago change skips cross-modulation arithmetic when its amount is zero. Both builds use the same direct 909 table-coordinate calculation. The second build starts audio DMA earlier, services audio interrupts during mandatory USB startup and starts the key scanner after UART setup. Measured application sizes on both are 546,516 B Elite and 546,508 B Pro. Each leaves 8,972 B of pool and 1,100 B of ordinary RAM free. Both cold-build source sets independently passed all 17 host scenarios and matched all 50 output files. [Source patches, hashes and model links](../../experiments/sound-hardware-v1/strict-timing/xmod-coordinate-builds.tar.gz)

| Shift | Chicago zero-XMod Elite (µs) | Chicago zero-XMod Pro (µs) | Earlier DMA start Elite (µs) | Earlier DMA start Pro (µs) |
| --- | ---: | ---: | ---: | ---: |
| Control | 10,899 | 10,909 | 11,100 | 11,102 |
| +1 ms | 10,536 | 10,594 | 11,104 | 11,108 |
| +2 ms | 10,892 | 10,684 | 11,093 | 11,096 |
| +3 ms | 10,591 | 10,899 | 11,064 | 11,091 |
| +4 ms | 10,893 | 10,886 | 10,963 | 10,991 |
| +5 ms | 10,663 | 10,638 | 10,943 | 10,860 |

The Chicago zero-XMod build met the numerical goal in **10/10 measured shifted shots**. The shifted maxima were **10,893 µs at +4 ms (716 µs headroom) on Elite** and **10,899 µs at +3 ms (710 µs headroom) on Pro**. Every listed result for this build had zero raw and scene late. Its fixed 8-second shots completed 647 halves. Holding the event times fixed and extending only the capture to 8,012 ms produced 648 halves on Elite +4 ms and Pro +3 ms, at the identical 10,893 and 10,899 µs maxima and zero late. The earlier-DMA build completed 650 and its shifted maxima were 11,104 µs at +1 ms (505 µs headroom) on Elite and 11,108 µs at +1 ms (501 µs headroom) on Pro.

**Physical input behavior remains a blocker.** The Chicago zero-XMod build changed the final project in 12/12 paired comparisons against stock. At +1 ms on Elite, only two project bytes changed, but one changes the programmed 909 rhythm. In a separate test through the production UI, an identical chord presented in different foreground batches removes step 16 or step 15 from an already full mid-tom row. Across offsets 0 through +5 ms, the numbers of changed bytes were 7, 2, 2, 14, 5 and 5 on Elite, and 9, 5, 7, 3, 6 and 3 on Pro. The UI stores one current held step for the whole row. Stock itself produces six distinct full-project hashes over the six offsets on each board. Qualification uses a stock comparison at the same offset. [Measured image, RAM, DAC and project identities](../../experiments/sound-hardware-v1/strict-timing/candidate-measurements.json)

### The changed clap and bass length

The +1 ms Elite trace matches the original measurement byte for byte in final RAM, flash, DAC, core-load CSV, log and scene. Stock and candidate used the same Elite model and identical physical inputs. The 16 white keys went down at 4,001 ms and were released at 4,041 ms. The next press selected closed hat at 4,056 ms. The candidate polled the all-up state at 4,054.315 ms, removing clap step 16 at 4,054.415 ms. Stock polled at 4,061.139 ms, saw both the white-key releases and the next voice selection together, and retained the clap. Bass B reached 35 on stock and 29 on the candidate before transport started. At the first K6 press, the UI recognized it within 38 µs across images, but the candidate delivered it to audio 5.731 ms earlier. [Unmodified physical-input traces](../../experiments/sound-hardware-v1/strict-timing/native-panel-plus1.tar.gz)

Moving the DMA start while keeping the stock DSP and key-scanner cadence produced zero raw late but left nine project bytes different. A separate build rendered every free half during USB startup and returned zero late over 650 renders. It measured 11,427 µs and left three changed bytes, the 808 and 303B mixer levels and 909 clap step 16. An ordinary run reproduced its traced RAM and audio. Both startup changes are diagnostic. [Startup, physical DAC and 264 programmed masks](../../experiments/sound-hardware-v1/strict-timing/startup-and-input.tar.gz)

A one-block startup design uses the real renderer during the mandatory USB reset and transfers the prepared audio to the first live DMA handoff. It services every hardware edge and restores stock's 648 completed application renders. The unchanged unshifted Elite stimulus produced every recorded drum note and arrangement byte. Two final mixer levels still differed: stock sampled 909 and 808 at 113 and 115, and the candidate sampled both at 117 before a bank switch. Separate observation reproduced the ordinary run's complete audio and debug data. The application size matched stock, but the loader package grew by 10 B on Elite and 6 B on Pro. [Physical DMA edges, clocks, buffer writes and fader traces](../../experiments/sound-hardware-v1/strict-timing/oneblock-startup-six-trials.tar.gz)

| One-block startup, stock DSP | Control | +1 ms | +3 ms |
| --- | ---: | ---: | ---: |
| Elite peak (µs) | 11,300 | 11,308 | 11,350 |
| Elite changed project bytes | 2 | 12 | 4 |
| Pro peak (µs) | 11,443 | 11,327 | 11,329 |
| Pro changed project bytes | 5 | 4 | 6 |

All six shots recorded zero raw and scene late. At +1 ms Elite still lost the clap; at +3 ms one kick moved. Pro retained its stock drum masks in all three shots, with differences in bass programming and mixer levels.

Keeping stock startup and changing only playback produced two additional +1 ms Elite controls. Skipping settled unity multiplication during Play measured 11,281 µs and retained the clap, but bass B ended at 32 instead of 35. Skipping zero sequencer decrements after the existing stopped return measured 11,316 µs and retained bass length 35, but lost the clap. Both had 648 halves, one raw late at boot and zero scene late. Both matched the stock first-kick and Play render frame and passed 17 fixed-input host scenarios. [Playback-only isolated source and traces](../../experiments/sound-hardware-v1/strict-timing/preplay-isolation.tar.gz)

### Playback changes tested against the original panel recording

At +1 ms on Elite, moving the MIDI accent conversion outside the per-drum loop changed 31 application bytes, all inside the sequenced drum-event routine. Every named executable address remained the same. The candidate reproduced all 36,420 project bytes, all 298 post-Play events and all 333,312 captured stereo frames, including the clap and bass B length 35. Its timing also matched stock at 11,323 µs, 648 halves, one raw late at boot and zero scene late. [Executable, event, state and audio comparison](../../experiments/sound-hardware-v1/strict-timing/unchanged-layout-control.tar.gz)

On unchanged stock Elite, the only unshifted halves above target were 417 and 603. In both cases a sequenced note brought Chicago B out of silence. Stock assigned Chicago and the 909 to the same core for the long span after the note, and moved the 909 one span later. A new planner retains the measured Chicago cost at note release and restores it when the next note starts. With stock startup and stock DSP it moved the 909 to the other core on the first long span. Half 417 fell from 10,964 to 10,257 µs and half 603 from 11,402 to 10,285 µs. The unshifted and +1 ms runs peaked at 10,285 and 10,295 µs with 648 halves, one boot late and zero scene late. Four and twelve project bytes differed respectively. This first version added 176 application bytes. [Target dispatch, cost and physical comparisons](../../experiments/sound-hardware-v1/strict-timing/note-cost-planner.tar.gz)

A separate startup correction transfers the prepared block to audio-owned storage so display writes cannot damage it when the audio clock stalls. The unshifted Elite run preserved 648 renders with zero late, reached 11,311 µs, and differed in three sound bytes. Enabling separate 909 arithmetic only on short playback spans reduced the peak to 11,127 µs, 218 µs above target, and increased the application by 2,816 B over stock. On an identical saved project, its 276 event times and 105,472 jointly on-time stereo frames matched the startup-only control. [Clock-stall, two-core and saved-project receipts](../../experiments/sound-hardware-v1/strict-timing/owned-startup-and-split-909.tar.gz)

The captured Chicago B owner has one moving oscillator gain and three gains held at zero. A specialized renderer updates all four phases and sync resets, and shares the general oscillator mixer with the held path. Its moving 256-frame span fell from 397.097 to 295.849 µs at zero cross-modulation, but rose from 504.478 to 596.085 µs with cross-modulation at 0.25. All 1,120,000 host differentials and 60 captured native state/audio vectors matched. The application occupied 368 B less than stock on both models. Original Elite scenes reached 11,093 µs unshifted and 10,979 µs at +1 ms. Both retained the original boot late and changed the physical project. [Component sources, two native links and scene receipts](../../experiments/sound-hardware-v1/strict-timing/chicago-single-oscillator.tar.gz)

### Note-cost planner and audio-owned startup

Combining the packed cost estimate, sparse Chicago renderer and audio-owned first-block handoff produced the exact production footprints of 546,516 B Elite and 546,508 B Pro. The complete packages stayed at 651,732 B and 651,730 B, both loaders matched stock byte for byte, and each model retained 8,972 B free pool. All 17 fixed-input scenarios and 50 state/audio files matched.

| Shift | Elite peak (µs) | Pro peak (µs) | Changed Elite project bytes | Changed Pro project bytes |
| --- | ---: | ---: | ---: | ---: |
| Control | 10,261 | 10,291 | 2 | 2 |
| +1 ms | 10,379 | 10,116 | 12 | 1 |
| +2 ms | 10,409 | 10,123 | 5 | 4 |
| +3 ms | 10,354 | 10,375 | 7 | 5 |
| +4 ms | 10,347 | 10,377 | 4 | 4 |
| +5 ms | 10,240 | 10,242 | 3 | 3 |

All 12 shots completed 648 halves and recorded zero raw and scene late. The shifted maxima were **10,409 µs Elite at +2 ms** and **10,377 µs Pro at +4 ms**, respectively 500 and 532 µs under the 10,909 µs target. Both +1 ms projects recorded bass B length 32 where their matched stock runs recorded 35. Elite also lost the programmed clap on step 16. [Complete candidate patch, two native links and all 12 physical records](../../experiments/sound-hardware-v1/strict-timing/note-cost-and-owned-boot.tar.gz)

On Pro +1 ms both images received the same physical encoder transitions and decoded and consumed three E2, 79 E4 and 79 E6 detents. The decisive E4 detent arrived at 4,723.550 ms on stock and 4,723.545 ms on the candidate. Stock consumed it together with the next E2 detent and switched from BREAK to bass B first. The candidate consumed E4 while BREAK was still selected and switched to bass B on its next pass. Stock's extra bass detent also triggered the existing acceleration one turn earlier. Those two effects account for 35 versus 32. Both external observer runs reproduced their respective full primary RAM, flash, DAC, core log, run log and scene. [Contact transitions, encoder reads, acceleration and target layouts](../../experiments/sound-hardware-v1/strict-timing/pro-1ms-encoder-causality.tar.gz)

With stock startup and the packed cost planner alone, Elite peaks were 10,290, 10,220, 10,410, 10,258, 10,334 and 10,298 µs across offsets zero through +5 ms. Pro peaks were 10,294, 10,244, 10,283, 10,274, 10,294 and 10,394 µs. Each run retained one boot late and zero scene late. Only Elite +4 reproduced the whole project and its physical audio still differed before Play. This standalone planner added 160 application bytes on both boards. [All 12 projects, native DAC, owner lifecycle and builds](../../experiments/sound-hardware-v1/strict-timing/packed-planner-stock-startup.tar.gz)

A foreground startup alternative prepares the first block during mandatory USB reset time. Its standalone links matched stock application and loader sizes on both boards. Combined with the same planner and sparse Chicago renderer it occupied 96 B less than stock per board. Elite +1 measured 10,359 µs and Pro +1 measured 10,186 µs with 648 completed halves and zero late. Both retained all stock drum masks on those shifts, but 11 and three project bytes still differed. Elite control and Pro +3 reached 10,261 and 10,358 µs, also with project differences. [Physical DMA, cold links, startup transfers and paired projects](../../experiments/sound-hardware-v1/strict-timing/foreground-handoff.tar.gz)

### Tests that preserve stock programming or audio

A playback-only cost forecast on Elite +1 ms reduced the peak from 11,323 to **10,231 µs**. All **36,420 project bytes and 333,312 captured stereo frames matched stock**. Both runs completed 648 halves with one raw boot late and zero scene late. The compiler places the planner in a dedicated section and resolves the private symbols from its own build. The planner restores the last active Chicago cost only after the current estimate falls below half that value. The native application grew by 180 B. An earlier version restored the forecast on every onset. It preserved the complete pre-PLAY audio, reached 10,287 µs and changed ten Sound bytes during playback. [Both native sources, projects, physical audio and paired fetch observations](../../experiments/sound-hardware-v1/strict-timing/playback-forecast-placement.tar.gz)

A separate startup change services a genuine pending audio interrupt during USB settling and hands the prepared block to the first live half. Its compact implementation occupies 64 B less than stock on each model, retains 8,972 B free pool and preserves the original loaders. Over Pro control and shifts +1 through +5 ms, the peaks were 10,392, 10,388, 10,042, 10,382, 10,258 and 10,272 µs. Every run completed 648 halves with zero raw or scene late. The corresponding counts of changed project bytes were 6, 0, 12, 2, 4 and 4. Elite +1 measured 10,355 µs and differed only in the programmed clap on step 16.

On Pro +1 all 298 post-PLAY note events matched at the same rendered frames over their common 134,656-frame window. The first physical audio difference was at frame 97,856, during audition. Of 54 audition events, four moved by one block. From the physical PLAY half through the end, 229 of 264 full DMA halves and 123,675 of 135,168 stereo frames were identical. The first changed half in that window was 584. The source and both stock-link comparisons remain experimental. [Firmware changes, fault controls, startup and sample-by-sample comparison](../../experiments/sound-hardware-v1/strict-timing/real-pending-irq.tar.gz)

Combining this startup with the dedicated-section playback forecast reached **10,215 µs on Elite +1 and 10,135 µs on Pro +1**, with 648 halves and zero raw or scene late. Both first differed from stock at physical frame 102,464. Elite changed 11 project bytes and Pro changed five. The clean startup alone reproduced the first audio difference on each board. [Combined firmware, both native builds, projects and full captures](../../experiments/sound-hardware-v1/strict-timing/first-irq-and-playback-forecast.tar.gz)

In separate startup controls, an enumerating USB host received the same 41,365 silent stereo frames and 938 packets from each image, with zero empty packets or underruns. Injecting the emulator's deterministic core-1 memory-read fault caused both images to select the one-core fallback. The startup candidate completed 97 halves with zero late and the stock boot had one late. [Original and candidate resource, fallback and audio comparisons](../../experiments/sound-hardware-v1/strict-timing/first-audio-irq-admission.tar.gz)

### Playback forecast with the original startup

All ten shifted runs met 10,909 µs with stock startup and the separately placed playback forecast. The shifted peak was **10,292 µs on Elite** (+2 ms) and **10,350 µs on Pro** (+2 ms). Each of the 12 runs including controls completed 648 halves with the original single raw boot late and zero scene late. Six projects matched. Elite +1 was the sole run on which the entire physical stereo capture also matched. This source adds 180 B on Elite.

| Shift | Elite peak (µs) | Pro peak (µs) | Changed project bytes, Elite / Pro |
| --- | ---: | ---: | --- |
| Control | 10,230 | 10,399 | 12 / 4 |
| +1 ms | 10,231 | 10,240 | 0 / 0 |
| +2 ms | 10,292 | 10,350 | 6 / 10 |
| +3 ms | 10,235 | 10,261 | 2 / 0 |
| +4 ms | 10,286 | 10,285 | 0 / 4 |
| +5 ms | 10,237 | 10,237 | 0 / 0 |

The actual first nonzero block on stock targeted the bank freed at 467.884800 ms. Its safe-write deadline was 479.490134 ms. The final word landed at 482.728136 ms, and the emulator played that bank at 491.095467 ms. Stock's following half remained silent. The audio-owned, protected-vector and fixed-vector startup versions all first played the tagged block around 502.70 ms. The silent factory startup used by the heavy scene concealed this difference. [Same-offset strict sweep](../../experiments/sound-hardware-v1/strict-timing/playback-forecast-full-sweep.tar.gz) · [Nonzero native output words and bank ownership](../../experiments/sound-hardware-v1/strict-timing/first-nonzero-bank-comparison.tar.gz)

Preparing the first block earlier and submitting it to the original bank before its deadline restored its physical delivery on edge 4. The new source then delivered the second block on edge 5. Stock's late startup had left edge 5 silent. The first user action now followed 151 application renders versus 150 on stock, and the full run completed 649 versus 648. With stock DSP the +1 ms peaks were 10,990 µs on Elite and 11,096 µs on Pro. Elite's project matched all 36,420 bytes, Pro changed bass B length from 35 to 32, and both physical audio streams first differed from stock at frame 79,424. The application grew by 160 B per model. A forced 23.5 ms handoff stall crossed two DMA boundaries and could leave a partial buffer while the parity-based late counter stayed zero. Normal timing, startup phase, fault handling and actual DAC delivery are all recorded. [Original-bank source, real tagged DAC and fault controls](../../experiments/sound-hardware-v1/strict-timing/original-bank-contiguous-startup.tar.gz)

### Playback of one identical saved project

Stock Elite performed a normal Stop and Save to a disposable emulator flash file. Both images subsequently loaded the exact same 36,420-byte project through normal firmware boot. A single Play press replaced all programming gestures for this supplemental comparison. Stock and the first exact-size candidate emitted the same 276 musical and control events at the same rendered frames over 123,392 frames. The DAC was bit-identical over the 207 corresponding halves that both completed by their DMA deadline, covering 105,984 stereo frames.

| Image | Worst block | Raw late | Late during playback test |
| --- | ---: | ---: | ---: |
| Stock Elite | 12,923 µs | 34 | 33 |
| First exact-size Elite candidate | 12,579 µs | 6 | 6 |

The continuous DAC capture differs at missed deadlines. These saved-project results use a different stimulus from the 922-event sweep above. [Save/load, stimulus, event and PCM proof](../../experiments/sound-hardware-v1/strict-timing/saved-project-and-events.tar.gz)

On the same saved project, the direct-coordinate candidate recorded 12,235 µs and four raw and scene late. Its 276 relative event times matched, but transport started 512 rendered frames earlier than stock under the same physical Play press. Even the shared on-time DAC windows differed. A separate diagnostic delayed only that Play press by 8 ms. Both then started at rendered frame 80,384 and all 105,984 comparable stereo frames were bit-identical. It measured 12,213 µs and four late. The original 922-event sweep retains its original physical inputs. [Saved-project, phase and physical chord replays](../../experiments/sound-hardware-v1/strict-timing/saved-project-and-events.tar.gz)

The Chicago zero-XMod build on the same saved project measured **12,222 µs with four raw and four scene late** under the identical physical Play press. All 276 event frames relative to transport matched stock. Starting transport one audio block earlier changed the DAC output. On its separate +8 ms Play diagnostic it measured 12,190 µs and four late. At the same absolute render frame all 105,984 commonly on-time stereo frames matched stock byte for byte. These runs use the saved-project stimulus, separate from the successful shifted timing sweep. [Source and full captured comparison](../../experiments/sound-hardware-v1/strict-timing/xmod-physical-and-playback.tar.gz)

The note-cost planner with audio-owned startup also loaded the same stock-saved project on Elite. Under the same single Play press it started at the stock transport frame of 80,384. All 276 events and all 105,984 commonly on-time stereo frames matched stock exactly. The continuous outputs first diverged 1,536 frames after transport. This separate, heavier scene peaked at 12,685 µs with ten raw and scene late, compared with stock at 12,923 µs, 34 raw and 33 scene late. [Boot readback, projects, event order and DAC comparisons](../../experiments/sound-hardware-v1/strict-timing/note-cost-and-owned-boot.tar.gz)

### S6 feedback and storage

A two-cursor experiment with four checkpoints needs 2,712 B per owner and returned 176,400 original wet words exactly with an ideal random-access dry source. The first wet callback still needs 49,662 source frames per owner. An optimistic 360 MHz kernel needed 19.769 ms and 23.582 ms for its two cursors, or 43.351 ms serially for one owner. Even with a core per cursor, the larger cursor exceeds 11.609 ms. The stock app and both 1 MiB flash images contained zero exact nonzero 128-sample source blocks in the tested formats. The live feedback, independent part histories and actual dry-source generation still need a fitting implementation. [Source, model receipts and replay](../../experiments/sound-hardware-v1/synthone-delay/checkpoint-provider-feedback.tar.gz)

Forecasting instead of waiting for the tap brought one owner's predicted wet half to 893 µs. Feeding 378 earlier frames per owner per callback completed the prewarming in time. One legal live delay-time edit at frame 242,550 then changed all 2,048 tap words on that owner. The peer stayed byte-identical. Exact regeneration for the edit took 23.211 ms with an ideal source, including 15.325 ms for the longer cursor alone. S6 remains FAIL. [Live edit and target states](../../experiments/sound-hardware-v1/synthone-delay/prewarm-live-edit-proof.tar.gz)

A follow-up computes only the feedback paths that each cursor's outputs depend on. Four checkpoints, two bounded cursors and their frame indices need 2,468 B per owner in the measured short-history witness. On the edited half, receiving precomputed tap indices and coefficients reduced one owner's arithmetic to 9,920 µs. Recomputing the tap positions and weights and writing all four wet streams in the actual requested order cost 16,148 µs on the target model. The unchanged peer took 1,228 µs. All 4,096 output words and 330 final state words matched the original instruments. Both measurements still grant the historical dry data. [Native code, exact outputs and replay](../../experiments/sound-hardware-v1/synthone-delay/split-cursor-proof.tar.gz)

Replacing the ideal dry provider with an eight-bin, note-periodic estimate needed 224 B for saved records and 300 B while encoding. On the edited owner's 512-frame half, its long cursor alone took 14,752 µs and the short one took another 8,157 µs. All 2,048 wet words differed on each owner, and both missed the predeclared 24 dB signal-to-noise requirement. A 64-bin control still missed that requirement with 1,532 B of state. [Original musical inputs, target ABI and results](../../experiments/sound-hardware-v1/synthone-delay/bounded-periodic-source-proof.tar.gz)

The complete two-reader state needs 2,732 B per owner including record metadata, leaving 32 B. Keeping the live encoder and both readers together takes 2,800 B, exceeding the 2,764 B budget by 36 B. [Measured PI32v2 combined layouts](../../experiments/sound-hardware-v1/synthone-delay/combined-storage-abi.tar.gz)

A second history encoder fits a 64-bin cycle and nine pitch corrections within each completed note. Its 16 packed notes use 1,552 B per owner. On the edited bass it improved the four wet-tap SNRs from 7.41–19.23 dB to 19.55–25.71 dB. On the second program all four stayed below 0 dB with tracking. All eight wet streams missed at least one of the original level, spectrum and 24 dB SNR criteria. This test allows offline note analysis. Native encoder memory and timing remain unqualified. [Packed records, controls and complete wet comparison](../../experiments/sound-hardware-v1/synthone-delay/tracked-note-source.tar.gz)

A two-core PI32v2 experiment generated the historical smooth controls during replay and synchronized the long and short workers on shared memory. For this one edit, one retained checkpoint and the zero start sufficed. With ideal historical dry input the edited and unchanged owners took 10,075.624 and 624.589 µs in the warm run. Each produced all 1,024 original final stereo words exactly after the authentic remaining effects. With bounded 8-bin or 64-bin historical audio the owners took 14,720.716 and 789.365 µs. The larger source, live encoder, retained state and workers occupy 2,716 B per owner, leaving 48 B for these particular histories. All 1,024 final words differed on each owner and every full stereo comparison missed the required fidelity. The final reverb and compressor ran separately from the timed worker. [Exact output, two-core timings, ABI and complete original graph](../../experiments/sound-hardware-v1/synthone-delay/dual-core-complete-wet-proof.tar.gz)

Fitting two independent tones to each completed note used 1,568 B of records per owner. All eight wet taps missed the original fidelity limits. As a diagnostic, supplying the original phase and amplitude trajectory for all seven voice slots improved several taps, but even the 64-bin fit passed only one of eight. On the second program, five voices were active together for 121,275 of the 176,400 recorded frames. The supplied phase data alone uses 29,635,200 B per owner. Both observed complete original traces matched their untouched controls. [Stored bytes, per-tap results and original-source replay](../../experiments/sound-hardware-v1/synthone-delay/note-source-and-voice-phase.tar.gz)

The original voice code can resume from **216 B per voice, or 1,512 B for seven slots**, on the PI32v2 ABI. Across 2,087 forced resets and restores per owner, both complete six-second traces matched all 5,292,000 original binary32 words each. Erasing oscillator phases or filter histories changed more than 2.4 million words on each program. The rest of the scheduler, shared random generator, control smoothers and effects stayed live. This qualifies one part of an independent historical reader. Its remaining state, complete native replay cost and S6 fit are open. [Voice checkpoint, positive and negative controls and target layout](../../experiments/sound-hardware-v1/synthone-delay/original-voice-checkpoint.tar.gz)

A separately initialized original scheduler now reproduces all **529,200 dry stereo words per owner** over the six-second score. Adding bitcrusher, pan, phaser and input-filter history also restored every word entering the delay. Maximum serialized records were **2,044 B on program 1 and 2,060 B on program 8**. For the actual abrupt edit, two saved points reproduced all 17,556 long-cursor and 9,034 short-cursor source frames, including a note change during replay. The long source alone used 18.409–32.171 ms of host thread CPU in five runs. The time excludes delay feedback and belongs to the host. [Original-input reconstruction, measurements and negative controls](../../experiments/sound-hardware-v1/synthone-delay/independent-original-input.tar.gz)

Across 4,176 original source records, a lossless compressor reproduced every byte. The two required checkpoints together used 1,205 B on program 1 and 1,987 B on program 8 when the second was stored as a difference from the first. A field-based predictor reduced program 8 to 1,788 B. On PI32v2 the six individual bytewise-compressed payloads expanded in 80.642–97.493 µs each. The decoded destination, active source state and two simultaneous readers still require space and processing. [Encoded records, per-block sizes, round trips and target decoder](../../experiments/sound-hardware-v1/synthone-delay/lossless-source-records.tar.gz)

Reassigning part of the right delay's work to the other core produced all four original wet taps and all four new feedback writes with supplied exact input. The edited owner took 9,406.277 µs. A bounded periodic source with a four-scalar read cache took 14,309.663 µs and changed the audio. Omitting its current right-channel feedback writes first changed a real output 13,406 frames later on the edited owner and 110,207 frames later on the peer. This rules out dropping that work simply because it is inaudible in the scored half. [Two-core source, deadline and future-output comparisons](../../experiments/sound-hardware-v1/synthone-delay/cascade-work-and-future-reads.tar.gz)

A PI32v2 run checked 169 active calls of the existing exact original voice renderer, including its integer-based double-precision helpers. All 676 output words matched. For the first 16 historical frames on the edited program, the native long and short voice workloads totaled 28,357.747 and 28,259.419 µs of single-core work. Even splitting this work perfectly across two cores leaves more than 14,129 µs per workload, before the control, stereo input filter or feedback work. This limit applies to this measured implementation. [Original dynamic states, target calls and returned samples](../../experiments/sound-hardware-v1/synthone-delay/target-original-voice-cost.tar.gz)

A size-neutral version of the guarded Chicago planner measured 10,289 µs on Elite +1 ms with one cold-start late. It retained the notes and arrangement, but ten Sound bytes changed. Output first differed at stereo sample 288,832. The target controls include 288 active effect/control cases. The optional EQ path cost 18.94% more when active. [Exact-flash source, trigger, audio comparison and effect controls](../../experiments/sound-hardware-v1/strict-timing/confined-planner-controls.tar.gz)

The selected Synth One part can reuse its retired Chicago insert's 4,096-byte buffer. With 2,764 bytes unused by the current Synth One backend, this gives 6,860 conditional bytes per part. Filling both buffers preserved the complete 36,420-byte project and all 92,672 stereo frames through real bank changes, notes and rendering. All 12 Chicago algorithms matched their pristine outputs for 14,336 frames each after a nonzero overwrite and cold re-selection. Rejected and pending requests retained the old owner. Both production ELFs reserve 8,504 bytes for the two insert objects and leave 8,972 pool bytes free. The complete historical reader still needs a live working-set and timing measurement. [Ownership fixture, target layouts and receipts](../../experiments/sound-hardware-v1/synthone-delay/selected-insert-ownership.tar.gz)

The separate zero-XMod Chicago optimization matched 840,000 host cases and all 60 target state and PCM cases. It saved 33.9 µs at 256 moving frames with zero XMod, and added 3.1 µs to one nonzero-XMod case. [Target differential](../../experiments/sound-hardware-v1/strict-timing/chicago-xmod-proof.tar.gz)

### Low-pass substitution across the 694 factory programs

Both channels of **694/694 programs** met the predeclared 1 dB level and 3 dB spectral limits under their original two-second stimuli. The largest absolute level difference was 0.000084 dB on Red Minor. The largest spectral RMS difference was 0.050950 dB on The Pad. All 66,624,000 stereo frames per arm were finite, 96 programs were bit-identical, and all 1,388 reference output hashes matched the historical manifest. The new low-pass replaces three voice call sites and both delay-input low-pass channels in the recovered comparison graph. Other effects still use the original implementation. Full production program selection, an independent complete effects graph and callback deadline qualification remain open. [Reproduction, scoring contract and all 694 program records](../../experiments/sound-hardware-v1/stock-dry/factory-lowpass-694.tar.gz)

The original 128-frame, 48 kHz callback bench ran 520,500 callbacks per arm across all 694 programs. On this shared host the original missed 31,005 wall and 635 thread-CPU deadlines. The replacement missed 30,151 wall and 806 thread-CPU deadlines. Both retained the original floating-point mode, had zero nonfinite output words and exited 1 under the zero-miss contract. The per-arm timings include all callbacks and cannot establish a speed difference between these serial host runs. [Callback contract, all 1,388 per-program rows and raw-trace hashes](../../experiments/sound-hardware-v1/stock-dry/factory-callback-694.tar.gz)

## Production Synth One INIT and original comparisons, 2026-11-21 UTC

The normal key, cell and render paths now play Synth One INIT on both owners. The former fixed one-sixth gain failed all eight original note/chord scenes. The original sums six voices at unity, and the corrected firmware matches that level. The existing Juno with chorus, Chicago and Miami still build. Older descriptions of a silent slot below refer to their historical checkpoints.

| Item | Status | Current evidence |
| --- | --- | --- |
| S1 | FAIL | INIT plays with independent voices, pitch bend, damper, note reuse and panic. The [compiled production selector and per-program inventory](../../experiments/sound-hardware-v1/factory-admission/README.md) show 0/694 complete original programs selectable. In [2,048 production callback blocks per scene](../../experiments/sound-hardware-v1/stock-dry/callback.md), dual INIT missed 0 CPU and 8 wall deadlines. Adding 808/909 missed 1 CPU and 8 wall deadlines. All samples were finite. Neither scene met the zero-miss contract. |
| S2 | FAIL | The [dry-note comparison](../../experiments/sound-hardware-v1/stock-dry/README.md) passed 180/180 static and 72/72 modulated notes at unity. Eight actual-part comparisons passed including two chords. [Oscillator controls](../../experiments/sound-hardware-v1/band_morph/README.md) passed 18/18 controlled notes. The 694-row inventory records empty metrics for unavailable full-program renders. |
| S3 | FAIL | The [direct AMP2 replay](../../experiments/sound-hardware-v1/sj6-amp2/README.md) proves four additional second-layer controls and owner isolation in the compact candidate. The packed macro drops these original fields. A [separate typed application entry](../../experiments/sound-hardware-v1/sj6-amp2/typed-entry.md) applied all four through the real audio block on both owners. It has no shipping bank importer. A separate pre-note waveform/PWM entry qualifies both layers on 474/640 authored programs, but its native link misses the required XIP reserve. All 64 controls and complete FX routing remain unverified. |
| S4 | BLOCKED | [Original inventory and capture contract](../sound/superj6-ab.md). The available original iOS binary requires a compatible host. The recovered 56 WAVs are drum assets and component comparisons. Zero of 14 complete original preset renders have verified stimuli and matching audio. |
| S5 | FAIL | All 640 SuperJ6 records across 12 banks decoded, with 90,549 source field rows preserved. The compact candidate applied four fields per program, 2,560 writes total. All 2,560 values also matched bit-for-bit after 320 actual block boundaries. The source has 640 distinct complete records and only 539 distinct AMP2 tuples. Full parameter binding and bank selection remain incomplete. |
| S6 | FAIL | The stock INIT image fits both models. Elite has 6,156 bytes free and Pro 6,164, with pool margin 780 on each. [Per-engine RAM/flash](sound-memory.md) and [original delay history counterexamples](../../experiments/sound-hardware-v1/synthone-delay/README.md). Full simultaneous effects and factory instruments have no passing physical image. |
| S7 | PASS | Case-insensitive whole-word scan of firmware, tests and web has zero matches. |
| C1 | FAIL | The canonical Local gate is running on source ca0c00a4e5863132de4a66f5d1c477c14d699be5. The full suite has recorded failures. Four focused reference reruns passed after restoring seven pinned source objects. The TouchOSC test passed with the verified upstream Lua 5.1 interpreter. The original key-sound test and its missing-cue control passed. The drum AMP test passed 238 normal checks, both graph controls and both caption mutants. Five browser safety checks passed. The full browser test still requires the pinned headless shell 1234, whose download returned HTTP 403. LeakSanitizer also recorded a ptrace permission failure in this environment. The original release-footer test also rejected both of its visual mutants. All 27 failures observed at the checkpoint came from test programs with identical Git blobs on consolidated and the lane. At its 318-case checkpoint the suite had 29 failures while the remaining generated-header cases continued. The later UI theme test passed 15,032,750 normal checks and 259 captures. Its sanitizer run encountered the same LeakSanitizer inspector restriction. The terminal gate log is pending. [Focused reruns and controls](../../experiments/sound-hardware-v1/gate-controls/independent-controls.tar.gz) Focused INIT, slot, legacy and Juno tests pass. All four original comparison checkouts are available at their exact pinned commits. The interrupted pre-change suite already recorded host failures, including LeakSanitizer terminating in multiple host binaries. |
| C2 | PASS | Consolidated `76040daa5b52d686a8b7b544296ba5d79610a95d` is an ancestor. No deleted paths in the no-rename comparison. |
| C3 | OPEN | No screen layout source changed. The designated independent release review has not signed off. |
| C4 | OPEN | New evidence consists of source, measurements and reproduction material. The exact-head gate owns the repository-wide limits receipt. |
| C5, C6 | FAIL | Checklist remains open. PR #973 stays in draft pending qualification and integration review. |

The [whole-app legacy controls](../../experiments/sound-hardware-v1/sj6-amp2/entry/README.md) include the failing compact-backend legacy test build transcripts. The [focused stock runtime log](../../experiments/sound-hardware-v1/stock-dry/freeze-test.log) and isolated oscillator model logs are retained with their source fixtures. The combined-head Local gate uses the four pinned original comparison repositories and a Pro package built locally from the frozen firmware. The shipped native pair and consolidated ancestry were rechecked for this evidence checkpoint.

### Independent Synth One low-pass

The original two controls that caused nonfinite output were replayed through the genuine Synth One parameter smoother and low-pass. Stock passed 68 of 75 fixed and changing-cutoff cases. Seven switched traces became nonfinite at the minimum 1 µs port half-time. The bounded candidate passed all 75 with worst level error 0.000610 dB and worst spectral error 2.094 dB. Existing original dry-note comparisons also passed, 180 static notes and 72 modulated notes. Independent cold links measured 546,676 B Elite and 546,668 B Pro, 160 B above each stock application. Each retained 8,972 B free pool. This is a separate component experiment. Full-program fidelity, source admission and the user's unchanged-flash timing requirement remain open for this source. [Alternate source, original captures and native builds](../../experiments/sound-hardware-v1/stock-dry/live-cutoff-proof.tar.gz)

### Remaining engineering work

The 32-byte cell exposes INIT and two live performance parameters. A factory importer needs a distinct typed interface for physical parameters, full validation before applying any ordered stream, and preservation of both owners when a program is rejected. App parameter IDs 0 and 1 represent oscillator morphs. Passing them through the existing performance API changes pitch bend and damper. The independent oscillator prototype separates the two domains. The [event review](../../experiments/sound-hardware-v1/stock-dry/event-review.md) also records DnBOX's paired-note-off convention for overlapping presses from the same source.

For SuperJ6 the tested typed entry now carries all four AMP2 values intact on both owners. The importer still needs a normal caller, stable bank/program identity and support for the remaining original fields. The existing packed cell cannot carry the complete programs. Complete original preset captures also need the matching iOS AU host or the authenticated 14/56 comparison archive. The delay comparison documents two distinct audible original histories, the measured available stock storage and the results of a [2,748-byte per-owner approximation](../../experiments/sound-hardware-v1/synthone-delay/bounded-delay.md) that failed on the actual wet mixes. An exact on-demand replay regenerated 44,100 delayed sample words in two zero-feedback scenes. Its cold request needs 22,136 frames per owner before the first requested tap. The [measured replay](../../experiments/sound-hardware-v1/synthone-delay/deterministic-replay.md) exceeds the callback budget. Complete-preset comparison and callback qualification on the full graphs remain open.

## Stock memory checkpoint, 2026-11-20

The [sound memory report](sound-memory.md) records six new physical links using the existing UI and normal stock linker. Playable Synth One with the shipped Juno leaves 4,160 B XIP on Elite and 4,168 B on Pro. The partial compact SuperJ6 variant leaves 1,864/1,872 B, or 1,928/1,940 B with the range dictionary. Each leaves 8,972 B raw pool, 780 B above reserve. Whole-lane S6 remains FAIL because complete independent instruments and insert histories are still absent. Source patches, attribution and native receipts are in the report.

The current lead task is to test the filter response on the original note recordings and verify both the factory parameter namespace and the working oscillator/control candidates. The complete branch gate and independent review are outstanding. The live base is `76040daa5b52d686a8b7b544296ba5d79610a95d`; GitHub and the local merge-base agree, and the no-rename diff contains no deleted paths. Scope and evidence prior to this checkpoint follow below.

## Independent SuperJ6 layer 1 attack field — 2026-11-19

The [SuperJ6 field adapter](../../experiments/sound-hardware-v1/sj6-id36-field-adapter/CURRENT.md)
now applies already-decoded app ID 36 (`neonattack`, main application position
34) to the first layer of the independent DnBOX candidate. It uses the existing
normalized attack conversion and physical setter. The caller must explicitly
establish independent, unlinked envelopes. Unknown or conflicting link state
rejects the complete attack batch without mutation. ID 119 remains raw-only.
The adapter adds no per-sample dispatch or persistent state.

The raw-only baseline reproduced the bug: at knob zero it stored the value but
left attack at 0.008 s instead of 0.002 s. Five normalized settings then matched
the direct physical path across all 10,240 stereo frames, including complete
part state; layer 2 remained audible and unchanged. Wrong-layer and linear-law
mutants failed. This establishes pre-note field application for one qualified
control. It does not establish linked-mode callbacks, the full preset importer,
original complete-preset audio, native size/cost or production activation.

## Measured stock fit and independent dry voice — 2026-11-19

Sole Sound ownership and existing `lane/sound` / draft PR973 are preserved.
The exact predecessor/partial-upload recovery completed before source writes,
as recorded below. Original engines are comparison references only. New sound
behavior uses existing DnBOX components and independently written adapters.
**Production Synth One remains silent and unresolved.**

### Actual native results

The separate local executor completed the original compact Elite pair, then
the four stock jobs allocated by Integ comment 6995594729. Stock results and
all four physical maps/command-log archives are published in
[`f291e916a9b61ae862899f0d26e21d42d76a7846`](https://github.com/gobbajogger/DnBOX/commit/f291e916a9b61ae862899f0d26e21d42d76a7846).
All 292 serial stock compilations, four links and 12 inspections exit0. Every
plan received fresh exact-input/toolchain/header/environment/capacity checks;
all owned local process groups were released and the allocation is consumed.

| Stock image | Load including marker | Above unchanged 1,024-byte XIP reserve |
| --- | ---: | ---: |
| Elite reference | 551,492 B | 156 B |
| Elite shared | 544,708 B | 6,940 B |
| Pro reference | 551,484 B | 164 B |
| Pro shared | 544,700 B | 6,948 B |

Sharing saves 6,784 bytes per stock model. The earlier compact Elite saving is
6,496 bytes; these different closures are not additive. Stock Pro does not
resolve unmeasured compact/full independent Pro fit. All four stock maps keep
pool raw free 8,972 bytes, 780 above 8,192 and 268 beyond the extra 512-byte gate
margin; ordinary RAM 1,100 free; XPOOL 0. Default development model 900 and the
empty BREAK bank stay explicit. Stock profiles still include silent S1.
[Detailed scope and exact evidence](../../experiments/sound-hardware-v1/shared_math/production_fit/README.md).

### Implemented dry-voice behavior and genuine failures

The [independent dry-voice package](../../experiments/sound-hardware-v1/dry_voice_followup/README.md)
now preserves four frozen variants. A passes 79/180 static notes; B 109/180;
C 109/180; D 138/180, 45/60 complete files. All full gates remain **FAIL**, with
zero nonfinite samples. D adds 29 passing notes relative to C and loses none;
all 45 zero-resonance notes remain byte-identical. D's worst RMS error is
4.832778 dB and worst spectral error 8.991580 dB, against unchanged 1/3 dB limits.
The selected 2 kHz/resonance 0.9 file now passes all three notes.

C corrects the delayed-feedback instability with centered feedback and adds
two history floats. Matched isolated filter observations show C compressing
signals where the original remains proportional at the three tested levels.
D replaces only that nonzero-resonance feedback with the existing primitive's
unfitted slope-five linear law and a direct implicit solve. It retains C's
complete r=0 branch, forward poles, coefficient preparation, histories, gain,
controls, two substeps and tail policy. No original algorithm was copied.

C/D use 160 host bytes per mono voice. Fresh two-owner tests each verify 8,192
sample words, 310 complete state checkpoints and 310 untouched-owner checks
under irregular blocks/live edits; 16,384 outputs are finite. Source reviews
and exact comparisons establish their bounded scope, not full instruments,
arbitrary modulation, target cost or production activation.

The remaining 42 static failures are at 64 Hz/resonance 0.9 for notes 36/84, and
8,000/22,050 Hz at resonance 0.5/0.9 for all three notes, across all three
velocities. Their exact reports remain failures. No tolerance, gain, alignment
or retirement rule was changed to hide them.

### Reference recovery and remaining qualification

All 60 static original files/180 notes were recovered exactly. Six zero-depth
dynamic pairs are authenticated aliases to static originals, providing 18
labels without new observations. Historical nonzero-depth paired PCM for 18
files/54 notes remains missing. A separately rebuilt original-only closure
reproduced the selected three-note baseline byte-for-byte, then produced one
new exact-contract active-filter original capture. Original engine source
stays isolated in comparison recovery evidence.

On this one new depth-1.2, cutoff-2,000 Hz, resonance-0.9 recording,
unchanged D passed all three notes after its static replay was byte-identical.
RMS/spectral errors in dB were 0.232227/0.682166 (36), 0.239112/0.767147
(60), and 0.196105/2.106154 (84), gain 1, zero nonfinite. This is three new
active observations, separate from the 138/180 static count. It does not
recover the historical paired files or qualify the 72-case modulation matrix.

The published comparison entry snapshots verified source/metric bytes, rejects
existing/symlink outputs, enforces exact note coverage and gain 1, and retains
mandatory checks under optimized Python. Original runners/failures remain
immutable evidence. All earlier exact envelope/component results are retained.

`scripts/gate.sh sound --post` remains unqualified and unrun. Target identity
still needs exact successful image/package pairs and a reviewed 909/808/BREAK/
FX/master driver. The available old 94197fc8 diagnostic is not that proof.
Full native callback timing, simultaneous complete S1/SJ6/FX memory, all 150
S1 controls/694 presets and 218 SJ6 controls/640 presets remain open. Web keeps
its shared RDC window; there is no fallback. No PSRAM, main/force/device,
Actions/Codex or #156 operation is introduced. PR973 remains draft.

## Active successor implementation — 2026-11-18

The user transferred sole Sound ownership from the length-limited thread
`6afe1680-84a4-83e9-99bc-62619d34b5d0`. Exact recovery finished before source
writes: all 690 staged paths and 342 reviewed files matched tree
`74fdcc425ba2e8bbf8d287ebaf3577d462ce048c`, with no deletions. The verified
preservation commit is `9be11840b11d256f22a9af19d290cfe96f31a429`, ordered parents
`b90faafd2de71063448beea67aa5d22938a5c64b` and consolidated
`76040daa5b52d686a8b7b544296ba5d79610a95d`. Existing `lane/sound` and draft
[PR #973](https://github.com/gobbajogger/DnBOX/pull/973) are retained.

The allocated separate local executor completed `compact-reference-elite-02`
and then `compact-shared-elite-03`, with every one of 74 compilation units
rebuilt for each member. The reference reproduced the 1,772-byte XIP reserve
shortfall. The shared-math physical link succeeded at **546,924 bytes**, saving
**6,496 bytes** and leaving **4,724 bytes above the unchanged 1,024-byte XIP
reserve**. Both retain pool raw free 8,972 bytes, only 780 above its 8,192-byte
reserve / 268 above the extra 512-byte gate margin. Ordinary RAM free is 1,100
bytes and XPOOL free is zero. [Actual results and evidence](../../experiments/sound-hardware-v1/shared_math/README.md)
are a partial-core resource measurement, not complete sound qualification.
Pro was not rerun; its prior 1,764-byte deficit remains open.

Implementation now includes an independently authored S1 envelope component
with 12-byte state, separate from SJ6's AHDSHR. Stage coefficients latch on
entry, sustain remains live, edits precede the sample, and retrigger retains
the current level. No allocation, transcendental calculation or shared mutable
state occurs in its sample routine. The component is not connected to the
production renderer. Its ten immutable scalar recordings total **379,288
binary32 outputs, all exact**, with five API/behavior tests, two input-integrity tests and four rejected
C mutants. A boundary-edit regression was added after independent review found
that an early decay latch survived the first four tests. All failed evidence
is retained. [Component contract and verification](../../experiments/sound-hardware-v1/component_followup/README.md)
explain the qualified scope.

Prepared-coefficient alternatives remain research-only. Existing fastmath is
exact on the recordings but differs for some legal physical controls against
the inferred mathematical formula. An independent integer proposal matched
5,360,032 tested coefficients, including subnormal boundaries, but still needs
native cost and broader original-behavior qualification. A recovered physical
range dictionary preserves complete state/audio in the host comparison; its
79-byte prior object saving is not an additional measured image saving.

Focused dispatcher tests `synthone-envelope-dnbx`, `fastmath`, and
`fastmath-ubsan` pass on the current source. The full firmware suite and
`scripts/gate.sh sound --post` have **not** run: exact production Elite/Pro
links, target regression/timing and the next local allocation remain open.
PR comment6994424328 requests the next bounded allocation. The two-link slice
is consumed, Web owns its next shared RDC window, and no RDC fallback is taken.

**Production Synth One is still silent and unresolved.** A complete independent
S1 profile still needs the common oscillator/VCA/four-pole filter path, all 150
controls, 694 presets, FX and two-part ownership; SJ6's 218 controls/640 presets
and full sound/FX qualification remain separate. Existing S1–S6/common FAILs
are not converted to PASS by these component results. No production activation
or consolidated merge is admitted. No PSRAM, main/force/device writes,
Actions/Codex work or #156 mutation is performed.

## Recovered predecessor checkpoint — 2026-11-18

This thread is the sole Sound production owner after consuming Dot Sound's frozen handoff. Preserve `lane/sound` and draft PR #973. The handoff archive SHA-256 is `4cc1b62ece0685eaa29b2c7ff86e7b58a4760a50b829bf27a18c38f6ef1bd773`; its live head was rechecked as `b90faafd2de71063448beea67aa5d22938a5c64b`. Consolidated `76040daa5b52d686a8b7b544296ba5d79610a95d` was merged in locally without conflicts; the existing lane history and files are preserved.

Latest user direction overrides the older implementation allowance: original SuperJ6 and Synth One engines are comparison references only. New code uses existing efficient DnBOX DSP and independently written behavior adapters. No external Codex or Actions jobs, PSRAM, device writes, main writes or force-push are authorized. There is one Sound writer.

The requested research logs are retained under RULES.md v2 rule 13; that rule and the current evidence-delivery request take precedence over the generic PR template's log exclusion. Firmware payloads and app installers are not new package contents.

The new [hardware-first research package](../../experiments/sound-hardware-v1/README.md) contains source, executable tests, independent review and retained failures. It is not wired into the production runtime. The [owner report](../sound/hardware-first-owner-report.md) records the complete evidence and gaps.

- Independent original-amplitude subset: 5,120 exact-timestamp endpoints pass the predeclared 0.001 threshold; worst error 7.95e-7. Shifted timing, linear contour and wrong velocity negatives reject. A real allocator defect was reproduced and fixed.
- Replacement RAM: both full part owners and existing selected FX remain simultaneously allocated. Candidate adapter plus stock chorus is 2,892 B inside the existing 3,720 B backend union; no pool growth is claimed or required by this partial core.
- Replacement flash: the compact reachable `-Oz` image is 553,420 B Elite / 553,412 B Pro. Physical overflow is 748/740 B; required-reserve shortfall is **1,772/1,764 B**. S6 remains FAIL, before the missing control/source/FX work.
- Callback: actual contract is 512/44,100 = 11.609977 ms. Candidate engine/event slice was measured, including event/control bursts and omitted render-counter work. Highest independent observed host maximum is 9.785132 ms. Full native callback, other simultaneous owners, stack high-water and xruns remain unmeasured.
- Synth One remains a silent production slot. The new common core does not match its four-pole transfer or VCA-before-filter topology. The historical original-code renderer remains reference-only; its existing fidelity results are not new-engine admission.

The production gates S1–S6 and common integration gates remain open or failing. No merge into consolidated is admitted by this checkpoint. Do not repeat the exhausted history-codec or compact-hall sweeps. The next work must reduce measured shared-code/adapter cost and independently qualify missing original behavior, with the whole simultaneous image accounted for.

## Preserved pre-handoff record

The current draft is PR #973 on `lane/sound`. This update restores the original generic Synth One renderer inputs and keeps its independent reference implementation separate from the experimental harmonic fixture.

## Checks on the restored source

- All 45 focused Python tests passed. Run `python3 -m unittest discover -s tests/host -p 'test_synthone*.py'`. Before restoration, three tests failed and seven raised errors.
- The portable bundle restored all 97 files. Its archive SHA-256 is `885f793f13845b74ded63151a65be75fdfadea79fc7696655397ea07bd498306`. The bundle and PCM manifest come from `f50d5c14`. The tuning fixtures come from `66b1ffef`.
- The pre-FX adapter rendered 576,000 frames across 12 scenes. There were zero differing channel words and zero render heap calls. The scalar control comparison checked 957,750 targets and found zero differing words.
- The generic factory comparison rendered 694 presets over 66,624,000 frames. There were zero differing or nonfinite words. Both outputs used the same FX implementation. All 1,388 output filenames, byte counts and SHA-256 hashes matched the historical PCM manifest. The commands in the portable input guide reproduce the test. [Factory results](../sound/restored-factory-replay.json) · [PCM manifest](../sound/v2-synthone-factory-pcm-manifest.json) · [Input guide](../../assets/test-inputs/synthone/README.md)

## Checklist

| Item | Status | Evidence or remaining work |
| --- | --- | --- |
| S1 | FAIL | The restored timing bench measured all 694 programs and missed 3,602 wall and 2,629 thread-CPU deadlines in 520,500 callbacks. All output words were finite. Profile the worst cases with the original floating-point settings. |
| S2 | FAIL | Historical harmonic A/B tests reported 492 bit-identical programs and 202 approximations. A current per-program spectral and level report remains open. |
| S3, S4 | FAIL | SuperJ6 control/FX coverage and comparison with the original captures have yet to be qualified on this lane. |
| S5 | FAIL | The replay covered 694 programs. Complete the independent control-to-parameter mapping check for the required banks. |
| S6 | FAIL | Last matched experimental pre-FX build exceeded Elite flash by 30,676 bytes and Pro by 30,668. Two-part pool margin was already 4,876 bytes below reserve before FX. Next measure retained code and independent voice histories against a matched stock build. |
| S7 | PASS | Case-insensitive whole-word Git scan under firmware, tests and web found zero hits on the restored source. |
| C1, C3–C6 | FAIL | Full firmware gate, independent review and overall checklist remain open. PR stays in draft. |
| C2 | PASS | The existing lane includes consolidated commit 89b40a93. No consolidated file was deleted. |

## Moved file

The experimental `firmware/src/dsp/synthone/scheduler/oracle/state.hpp` moved to `tests/fixtures/synthone-storage/oracle/state.hpp`. The generic renderer refuses to overlay any runtime source on its independent reference. Moving the fixture preserves that check and lets the normal renderer run against the pinned source.

## Timing and memory measurements

The callback benchmark and its original deadline contract were recovered from `55e1288d`. The untraced 48 kHz host test used 128-frame blocks. The full 694-program result was DEADLINE_FAIL. [Timing receipt](../sound/restored-callback-result.json)

At 44.1 kHz the four original ten-second delay histories alone allocate 7,056,016 bytes. In a fresh component test with pseudorandom input and a two-second delay, reducing the ring from ten seconds to 5.0001 seconds changed an output word at frame 220,528. An earlier minimum-size native build with link-time optimization still exceeded Elite XIP by 25,528 bytes and Pro by 25,520. The current build flags and effect behavior are preserved.
