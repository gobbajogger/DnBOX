"""Compare the complete wet 909 demo against the original engine and WAV files."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import tarfile

import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
REF = '10cbe5c1e1c9e65b0818bb34644ab1a13092082c'
sys.path.insert(0, str(HERE.parent/'sj6-factory'))
from compare import channel_score


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('upstream', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    original = out/'upstream'
    original.mkdir()
    tar = out/'upstream.tar'
    subprocess.run(['git','-C',args.upstream,'archive',REF,'-o',tar,'src'],check=True)
    with tarfile.open(tar) as archive:
        archive.extractall(original,filter='data')
    tar.unlink()
    binary = out/'build'
    env = dict(os.environ, NINEW9_DIR=str(original), D9_PACKED_DELAY="1", D9_TEST_OUT=str(binary), D9_DEMO_WAV=str(out/'matched.wav'))
    matched_log = out/'matched-source.log'
    with matched_log.open('wb') as log:
        matched = subprocess.run(['sh','tests/host/run_drum909.sh'],env=env,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT)
    summary = r'(?:PASS|FAIL): (\d+) failure\(s\)'
    matched_counts = re.search(summary, matched_log.read_text())
    if matched.returncode not in (0,1) or not matched_counts:
        raise RuntimeError('matched-source build or bench did not complete')
    pair = out/'pair.f32'
    command = [binary/'drum909_test', original/'src', out/'demo.wav']
    result = subprocess.run(command,cwd=ROOT,env=dict(env,D9_DEMO_PAIR=str(pair)),stdout=subprocess.PIPE,check=False)
    (out/'original-wav.log').write_bytes(result.stdout)
    report = result.stdout.decode()
    counts = re.search(summary, report)
    if result.returncode not in (0,1) or not counts:
        raise RuntimeError('original-WAV bench did not complete')
    pcm = np.fromfile(pair,dtype='<f4').reshape(2,429619)
    if not np.isfinite(pcm).all() or not np.count_nonzero(pcm[0]):
        raise RuntimeError('invalid wet four-bar pair')
    ref, port = pcm.astype('d')
    level, spectral = channel_score(ref,port,44100,[20,16000])
    digest = lambda data: hashlib.sha256(data).hexdigest()
    row = {'original':REF,'sample_rate':44100,'frames':len(ref),'channels':1,'reference':'original WAVs',
           'source_wav_sha256':{p.name:digest(p.read_bytes()) for p in sorted((original/'src/samples').glob('*.wav'))},
           'reference_sha256':digest(pcm[0].tobytes()),'candidate_sha256':digest(pcm[1].tobytes()),
           'relative_rms':float(np.linalg.norm(port-ref)/np.linalg.norm(ref)), 'level_db':level,
           'spectral_rms_db':spectral,'pointwise_suite_failures':int(counts.group(1)),
           'matched_input_suite_failures':int(matched_counts.group(1)),
           'pass':bool(abs(level)<=1 and spectral<=3)}
    (out/'results.json').write_text(json.dumps(row,indent=2)+'\n')
    print(json.dumps(row,indent=2))
    return int(not row['pass'] or row['matched_input_suite_failures'] != 0)


if __name__ == '__main__':
    raise SystemExit(main())
