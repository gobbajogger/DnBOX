"""Compare BB Gen and ordinary BREAK at three fractional playback rates."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import subprocess
import tempfile
import wave

import numpy as np

import replay


def write_reference_wav(adpcm, frames, expected_hash, path):
    words = np.asarray(replay.decode(adpcm.read_bytes(), frames), dtype='<i2')
    raw = words.tobytes()
    if expected_hash and hashlib.sha256(raw).hexdigest() != expected_hash:
        raise ValueError(f'Decoded audio checksum differs for {adpcm.stem}')
    with wave.open(str(path), 'wb') as wav:
        wav.setparams((1, 2, 44100, 0, 'NONE', ''))
        wav.writeframes(raw)


def compare_rate(executable, temporary, encoded, original, source, frames, speed):
    count = math.floor((frames // 8) / speed) - 2
    output = temporary / 'pair.pcm'
    dry = None
    for setting, divisions in zip(('off', '0', '1', '2', '3'), (1, 2, 3, 4, 8)):
        subprocess.run([executable, str(frames), str(count), encoded, original, output,
                        temporary, '1', setting, str(speed)], check=True)
        pair = np.fromfile(output, dtype='<i2').reshape(count, 2)
        output.unlink()
        if not np.all(np.any(pair != 0, axis=0)):
            raise ValueError(f'Silent render for {encoded.stem}, {speed}x, {divisions} subdivisions')
        if dry is None:
            dry = pair
        elif not np.all(np.any(pair != dry, axis=0)):
            raise ValueError(f'Unchanged roll for {encoded.stem}, {speed}x, {divisions} subdivisions')
        reference, candidate = pair.astype(float).T
        level, spectral = replay.channel_score(reference, candidate, 44100, [20, 16000])
        yield dict(loop=encoded.stem, source=source, speed=speed, divisions=divisions,
                   frames=count, level_db=level, spectral_rms_db=spectral,
                   passed=abs(level) <= 1 and spectral <= 3,
                   original_sha256=hashlib.sha256(pair[:, 0].tobytes()).hexdigest(),
                   firmware_sha256=hashlib.sha256(pair[:, 1].tobytes()).hexdigest(),
                   pcm_sha256=hashlib.sha256(pair.tobytes()).hexdigest())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('bb_gen_tree', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    replay.build(args.bb_gen_tree.resolve(), out)
    bank = replay.REPO / 'assets/factory-breaks'
    rows = []
    with tempfile.TemporaryDirectory(dir=out) as work:
        temporary = Path(work)
        for name, frames, checksum in replay.fixtures(bank):
            encoded = bank / (name + '.adpcm')
            decoded = temporary / (name + '.wav')
            write_reference_wav(encoded, frames, checksum, decoded)
            inputs = [('decoded', decoded)]
            original = bank / (name + '.wav')
            if original.exists():
                inputs.append(('original_wav', original))
            for source, wav in inputs:
                for speed in (0.75, 1.25, 1.5):
                    rows.extend(compare_rate(out / 'compare', temporary, encoded, wav, source, frames, speed))
    (out / 'tempo-results.json').write_text(json.dumps(rows, indent=2) + '\n')
    rolls = [row for row in rows if row['divisions'] > 1]
    dry = [row for row in rows if row['divisions'] == 1]
    for name, subset in [('Programmed rolls', rolls), ('Dry tempo controls', dry)]:
        print(f"{name}: {sum(row['passed'] for row in subset)}/{len(subset)}")
    return 0 if len(rolls) == 84 and all(row['passed'] for row in rolls) else 1


if __name__ == '__main__':
    raise SystemExit(main())
