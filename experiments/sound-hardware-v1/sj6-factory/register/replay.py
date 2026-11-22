#!/usr/bin/env python3
"""Read the archived register column and exercise it through both factory loaders."""
import csv
import hashlib
import io
import os
from pathlib import Path
import struct
import subprocess
from tempfile import TemporaryDirectory

ROOT = Path(__file__).resolve().parents[4]
SOURCE = 'bd60550833daa862ed5aca48e7af6b3d8d361de2:assets/superj6/original-fit-export/per_preset_metrics.private.csv'
SHA256 = '14fd2646763d3b718c307e9b8cd59e4876351b56f162bbf74f439e2c263259c8'


def main():
    raw = subprocess.check_output(['git', 'show', SOURCE], cwd=ROOT)
    if hashlib.sha256(raw).hexdigest() != SHA256:
        raise ValueError('archived metrics checksum differs')
    rows = list(csv.DictReader(io.StringIO(raw.decode())))
    if len(rows) != 640 or len({(row['bank'], row['i']) for row in rows}) != 640:
        raise ValueError('expected 640 distinct bank/index rows')
    words = [struct.unpack('<I', struct.pack('<f', float(row['osc_range'])))[0] for row in rows]
    with TemporaryDirectory() as tmp:
        path = Path(tmp) / 'register-words'
        path.write_text(''.join(f'{word:08x}\n' for word in words))
        subprocess.run(['sh', 'tests/host/superj6_factory_load_test.sh'], cwd=ROOT, check=True,
                       env={**os.environ, 'SJ6_REGISTER_WORDS': str(path)})


if __name__ == '__main__':
    main()
