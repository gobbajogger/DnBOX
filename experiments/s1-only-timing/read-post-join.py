"""Read separate held-window diagnostic timing bins; never for qualification."""
import struct
import subprocess
import sys
from pathlib import Path

run = Path(sys.argv[1])
assert 'skipped 0 cycles' in (run / 'run.log').read_text()
symbols = {
    p[7]: int(p[1], 16) - 0x01c00000
    for line in subprocess.check_output(['readelf', '-Ws', run / 'bench.elf'], text=True).splitlines()
    if len(p := line.split()) >= 8 and p[3] == 'OBJECT'
}
ram = (run / 'ram.bin').read_bytes()
read = lambda name, n: struct.unpack_from(f"<{n}I", ram, symbols[name])
assert read('bench_s1_only', 9) == (0, 65536, 12, 98304, 163840, 6, 0, 0, 0)
assert read('bench_loaded_hits', 2) == (132, 132)
assert read('bench_s1_held', 4)[0] == 128
for name, ticks in zip(
    ('Post-join / send shaping', 'FX + master wall time', 'Block setup',
     'Between-stretch sequencer', 'Deferred master', 'Output conversion',
     'Scope and meters', 'Entire engine render'),
    read('bench_post_cycles', 8),
):
    print(f'{name:28s} {ticks / (24 * 128):9.2f} us per 512 generated frames')
