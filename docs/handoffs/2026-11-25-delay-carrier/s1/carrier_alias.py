"""Same centered rectangular accumulators and linear reconstruction as the patch.
A pure steady-state input removes preset and feedback from the comparison."""
import numpy as np
sr, dd, source = 44100, 45, 3000
n = np.arange(sr * 2)
x = np.exp(2j * np.pi * n * source / sr).real
groups = x.reshape(-1, dd)
idx = n // dd
phase = (n % dd) / dd
def db(amp):
    return 20 * np.log10(max(abs(amp), 1e-13))
def amp(x, hz, rate):
    return 2 * np.abs(np.dot(x, np.exp(-2j*np.pi*hz*np.arange(len(x))/rate))) / len(x)
print('3000-Hz unity input at 44.1 kHz; store one cell per 45 frames, at 980 Hz.')
print('width   cell alias 60 Hz   linearly reconstructed 60 Hz   image at 3000 Hz')
for frac in (1,2,4):
    skip = (dd-dd//frac)//2
    cells = groups[:, skip:dd-skip].mean(axis=1)
    out = cells[idx] * (1-phase) + cells[(idx+1)%len(cells)] * phase
    first = db(amp(cells,60,sr/dd))
    lo = db(amp(out,60,sr))
    hi = db(amp(out,3000,sr))
    print(f'{dd-2*skip:5} {first:17.3f} dB {lo:23.3f} dB {hi:18.3f} dB')
    assert abs(lo-first) < 0.12
    assert -68 < hi-lo < -67
