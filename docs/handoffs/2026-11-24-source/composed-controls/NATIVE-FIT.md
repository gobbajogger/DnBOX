# Native guarded fit receipt

Full authored objects compiled on both models before either fit or behavior checks. The later exact native v4 links report:

```
a0d7b4e4431ba10b902e29a305d0961c10626a47e079fa45db98677a383f4c73
guarded elite XIP 552408 free 264 pool usable 632 read-only FDATA 61328 diagnostic slot3 span 32292 slot3 free 476 read-only FITTAB 3929 header 44395244020000000100000070ef000036162a599dcbddf328e5da215ef79317 ELF fc452aa09337611e24a56ba8f17489f0cd36df733a01c2a10a733fb7643b59a8
guarded pro XIP 552396 free 276 pool usable 632 read-only FDATA 61328 diagnostic slot3 span 32292 slot3 free 476 read-only FITTAB 3929 header 44395244020000000200000070ef000036162a5962c979e2f4e32c1efe753c2f ELF bef470692f261cbabb4c5bd62b96d30d1e3a15c09c40098eaeee3260169624a8
```

App extent includes packed offsets and the final marker. No PSRAM. The pool figures are above the 8192-byte reserve. Read-only FDATA, Slot 3, and FITTAB are sealed to the image. Never flash or publish these diagnostic images. Performance, behavior, and release qualification are not yet established.
