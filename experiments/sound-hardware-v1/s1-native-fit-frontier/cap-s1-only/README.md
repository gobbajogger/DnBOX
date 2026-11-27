# Synth One-only delay budget: 15,716 bytes per owner

Upper-bound experiment: 6,784 B existing lease plus half of 17,864 B measured
diagnostic spare pool. The diagnostic omits expanded controls; this is not
product memory allocation. The existing portable delay predictor used
15,628 B (13,668 B history payload + 1,960 B state allowance).
Only the delay is replaced in the already-authenticated source graph.

The new bound still fails authored audio: Pad -21.33 dBr signed, Hall
-18.02 dBr, Deeper -32.60 dBr; Deeper diverges 11.22 / 13.80 dB spectrally.
The same sanitized warm retime fixture accepts 93,055 updates and rejects
259,745 when sizing cold. Reserving the full 2.4 s accepts all updates
but limits the stored cadence to 2.104 kHz and fails the signed comparison.
Do not adopt or spend time on the full bank sweep for this design.
It is not a general impossibility proof.

Reproduce from the source-bound workspace with the bench directory as a
sibling of s1-portable-delay, and its authenticated source closure available:
```sh
d=cap-s1-only
python "$d/build.py" "$d"
OPENBLAS_NUM_THREADS=1 S1_SHAPES=0,1024 python s1-portable-delay/compare.py "$d" "$d/out" 62 151 366
```
Use the existing warm.sh compiler command with this directory's ring.c and
the two modes in the receipts. No device, app publication or realtime claim.
