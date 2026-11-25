# LFO1 to original Sequencer and Arp

Apply qualification-lfo1-seq.patch on top of the previous composed controls. An isolated real test of the original selector paths identified a missing LFO1-to-scheduler receiver. Add it without changing the LFO2/3 order. Full native object build on both boards succeeded before the exact fit below.

```
f18e867c9da9d64b5d60674c9bbb07ae3973fdd8ca960e331b6457c23b7dd280
guarded elite XIP 552536 free 136 pool usable 632 read-only FDATA 61328 diagnostic slot3 span 32292 slot3 free 476 read-only FITTAB 3929 header 44395244020000000100000070ef000036162a5993c4ffaa1309e389c888eb91 ELF 5de29f3096120b22d730a3a7957421f3eef4fbcb6dea52c967c2257117da25d1
guarded pro XIP 552524 free 148 pool usable 632 read-only FDATA 61328 diagnostic slot3 span 32292 slot3 free 476 read-only FITTAB 3929 header 44395244020000000200000070ef000036162a590d0b89840ae0407238242b61 ELF 021b407421042f3c9faab9d2c1d618a60af6858e764cdce6dad811a43e265f90
```

Exact runtime and loaded-audio timing remain pending. The old guarded board booted but the unpopulated raw replay had silent DAC; it is not a timing pass. Do not package, install or publish these read-only laboratory images.
