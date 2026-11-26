# Heavy scene attribution probe rejected
Instrumented the H16 synth render job to retain per-half counters using already
sampled ticks. First link moved the project layout and was correctly rejected
without launch. Moving the diagnostic state to the tail of .bss preserved the
original project object address and exact guarded overlay completed.
The candidate changed PCM by frame 65,600, changed USB timing and increased
dropouts from 0 to 79 by 4.7 s and 1 to 86 by 4.85 s despite zero emulator skips.
Its peak-half tuples are therefore not valid attribution for the baseline.
Do not adopt or infer which DSP stage caused the original late blocks.
