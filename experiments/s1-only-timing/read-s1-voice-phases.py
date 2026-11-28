"""Read stratified Synth One voice phase timing. Probe overhead is included."""
from pathlib import Path
import struct
import subprocess
import sys

d = Path(sys.argv[1])
assert 'skipped 0 cycles' in (d / 'run.log').read_text()
tab = {}
for ln in subprocess.check_output(['readelf','-Ws',d / 'bench.elf'], text=True).splitlines():
    f = ln.split()
    if len(f) >= 8 and f[3] == 'OBJECT':
        tab[f[7]] = int(f[1],16) - 0x1c00000
ram = (d / 'ram.bin').read_bytes()
read = lambda n,k: struct.unpack_from(f'<{k}I',ram,tab[n])
assert read('bench_s1_only',9) == (0,65536,12,98304,163840,6,0,0,0)
assert read('bench_loaded_hits',2) == (132,132)
cuts = read('bench_voice_cycles',4)
voice = read('bench_voice_count',1)[0]
control = read('bench_control_count',1)[0]
assert voice > 65536, voice
assert .11 < control / voice < .14, (voice,control)
print(f'Sampled {voice} held voice-samples; {control} actually refreshed control rate')
for label,ticks in zip(('ADSR','control','oscillators + noise','filter + mix'),cuts):
    print(f'{label}: mean {ticks/voice:.2f} ticks per voice-sample; approx {ticks/voice*6*512/24:.1f} µs per 512 frames including probe overhead')
