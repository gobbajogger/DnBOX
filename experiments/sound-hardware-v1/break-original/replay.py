"""Compare straight 44.1 kHz BREAK playback with the original BB Gen plugin."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import wave

import numpy as np

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]
BB_GEN = 'c3ea85da9025a3694c167e096cb09d0efb60420c'
sys.path.insert(0, str(REPO / 'tests/host'))
from cymbal_ref import decode
sys.path.insert(0, str(HERE.parent / 'sj6-factory'))
from compare import channel_score


def build(bb, output):
    for name in ('breakbeat.c', 'slice_select.c', 'slice_select.h', 'plugin_api_v1.h'):
        path = 'src/dsp/' + name
        ref = subprocess.check_output(['git', '-C', bb, 'show', BB_GEN + ':' + path])
        if (bb / path).read_bytes() != ref:
            raise ValueError(f'BB Gen differs from pinned source: {name}')
    upstream = bb / 'src/dsp'
    dsp = REPO / 'firmware/src/dsp'
    subprocess.run(['cc', '-std=gnu11', '-O2', '-fno-fast-math', '-ffp-contract=off',
                    '-DX0X_HOST', '-DX0X_IMA_TABLES_DEFINE', '-I' + str(upstream), '-I' + str(dsp),
                    HERE / 'straight.c', upstream / 'breakbeat.c', upstream / 'slice_select.c',
                    dsp / 'breaks.c', '-Wl,--wrap=mkdir,--wrap=symlink,--wrap=unlink',
                    '-lm', '-o', output / 'compare'], check=True)


def fixtures(bank):
    for row in json.loads((bank / 'provenance.json').read_text())['loops']:
        yield row['name'], row['prepared_frames'], row['decoded_pcm_sha256']
    for name in ('909FL', '909GR'):
        with wave.open(str(bank / (name + '.wav'))) as wav:
            if (wav.getframerate(), wav.getnchannels(), wav.getsampwidth()) != (44100, 1, 2):
                raise ValueError(f'Unexpected original WAV format for {name}')
            yield name, wav.getnframes(), None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('bb_gen_tree', type=Path)
    parser.add_argument('output', type=Path, help='fresh output directory')
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    build(args.bb_gen_tree.resolve(), out)
    rows = []
    bank = REPO / 'assets/factory-breaks'
    for name, samples, digest in fixtures(bank):
        adpcm = bank / (name + '.adpcm')
        prepared = np.asarray(decode(adpcm.read_bytes(), samples), dtype='<i2')
        if digest and hashlib.sha256(prepared.tobytes()).hexdigest() != digest:
            raise ValueError(f'Decoded source PCM checksum differs for {name}')
        decoded_wav = out / (name + '.wav')
        with wave.open(str(decoded_wav), 'wb') as wav:
            wav.setparams((1, 2, 44100, 0, 'NONE', ''))
            wav.writeframes(prepared.tobytes())
        sources = [('decoded', decoded_wav)]
        if (bank / (name + '.wav')).exists():
            sources.append(('original_wav', bank / (name + '.wav')))
        for kind, wav_path in sources:
            for s in range(8):
                frames = samples // 8 - 128
                pcm = out / f'{name}-{kind}-{s}.pcm'
                subprocess.run([out / 'compare', str(samples), str(frames), adpcm, wav_path,
                                pcm, out, str(s)], check=True)
                pair = np.fromfile(pcm, dtype='<i2').reshape(-1, 2)
                # DnBOX distributes the division remainder over all eight slices.
                # BB Gen leaves it at the end. This displacement comes only
                # from the input length and requested slice, before comparison.
                shift = (s * samples) // 8 - s * (samples // 8)
                skip = 89  # 88 gain increments plus their binary32 rounding
                orig = pair[skip + shift:, 0].astype(np.float64)
                candidate = pair[skip:frames - shift, 1].astype(np.float64)
                if len(pair) != frames or not orig.any() or not candidate.any():
                    raise ValueError(f'Incomplete or silent comparison for {pcm.name}')
                diff = orig - candidate
                level, spectral = channel_score(orig, candidate, 44100, [20, 16000])
                rows.append(dict(name=name, source=kind, slice=s, frames=frames,
                                 post_fade_frames=len(diff), boundary_difference_frames=shift,
                                 post_fade_different_words=int(np.count_nonzero(diff)),
                                 max_error_pcm16=int(np.max(np.abs(diff))),
                                 level_db=level, spectral_rms_db=spectral,
                                 pcm_sha256=hashlib.sha256(pcm.read_bytes()).hexdigest()))
    (out / 'results.json').write_text(json.dumps(rows, indent=2, allow_nan=False) + '\n')
    for kind in ('decoded', 'original_wav'):
        cases = [r for r in rows if r['source'] == kind]
        print(kind, len(cases), 'cases,', sum(r['post_fade_frames'] for r in cases),
              'frames,', sum(r['post_fade_different_words'] for r in cases), 'differing words')
    # The shared-source playback check is exact. Original WAV scores quantify
    # the lossy encoding separately and use the sound lane's 1 dB/3 dB limits.
    return int(any(r['post_fade_different_words'] for r in rows if r['source'] == 'decoded') or
               any(abs(r['level_db']) > 1 or r['spectral_rms_db'] > 3 for r in rows if r['source'] == 'original_wav'))


if __name__ == '__main__':
    raise SystemExit(main())
