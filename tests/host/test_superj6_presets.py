#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""SuperJ6 factory presets: all 640 programs in 12 banks load with a verified mapping.

The bank export is private (it comes from the owner's app copy) and is never
committed. Point SJ6_PRESETS at the J6presets directory or J6presets.zip; the
default is the box copy. Without it the test prints SKIP and exits 0.

Checks: the 12 bank files match the pinned hashes (sj6-amp2/bank-hashes.json);
the C bank reader (experiments/sound-hardware-v1/sj6-factory) selects every
program by UUID and by position and returns every authored binary32 field in
order; every serialized field name maps to an engine control: one of the
public catalog controls, or, for the two legacy names, an explicit limiter
stage input (the capture-fitted limiter model, Orchestrator decision 23 Nov).
Prints presets=640 unmapped=0."""
import json, os, shutil, sys, tempfile, zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FACTORY = ROOT / 'experiments/sound-hardware-v1/sj6-factory'
CATALOG = ROOT / 'experiments/sound-hardware-v1/mapping/results/implementer_brief/public_control_catalog.json'
DEFAULT = Path('/workspace/sj6-private/banks/J6presets')
# Legacy names outside the app's control list: inputs of the limiter stage
# (KoFMInstrument.limiter, a RageCompressor). Never folded into the compressor.
LEGACY = {'neonlimiterAmount': 'limiter.amount', 'neontubeRageAmount': 'limiter.rage'}


def archive_from(source, work):
    if source.is_file():
        return source
    banks = sorted(source.glob('*.superj6.bank.json'))
    out = work / 'J6presets.zip'
    with zipfile.ZipFile(out, 'w') as z:
        for b in banks:
            z.write(b, 'J6presets/' + b.name)
    return out


def main():
    source = Path(os.environ.get('SJ6_PRESETS', DEFAULT))
    if not source.exists() or (source.is_dir() and not any(source.glob('*.superj6.bank.json'))):
        print(f'superj6-presets: SKIP (private bank export not present at {source})')
        return 0
    sys.path.insert(0, str(FACTORY))
    from factory import load_factory
    from verify_bank import verify
    catalog = json.loads(CATALOG.read_bytes())
    known = {c['identifier']: c['id'] for c in catalog['controls']}
    if sorted(catalog['unknown_legacy_keys']) != sorted(LEGACY):
        print('FAIL legacy key list changed'); return 1
    work = Path(tempfile.mkdtemp(prefix='sj6-presets-'))
    try:
        archive = archive_from(source, work)
        summary = verify(archive, work / 'reader')
        factory = load_factory(archive)
        presets = fields = 0
        unmapped, legacy_used, nonfinite = set(), {}, 0
        for _, programs in factory.values():
            for record in programs.values():
                presets += 1
                for f in record['parameterValues']:
                    fields += 1
                    name, value = f['identifier'], f['value']
                    if not isinstance(value, (int, float)) or value != value or abs(value) == float('inf'):
                        nonfinite += 1
                    if name in LEGACY:
                        legacy_used[name] = legacy_used.get(name, 0) + 1
                    elif name not in known:
                        unmapped.add(name)
        ok = (presets == 640 and len(factory) == 12 and summary['programs'] == 640 and
              summary['ordered_binary32_checked'] == fields and not unmapped and not nonfinite)
        print(f'superj6-presets: presets={presets} banks={len(factory)} fields={fields} '
              f'unmapped={len(unmapped)} nonfinite={nonfinite} legacy=' +
              ','.join(f'{k}->{LEGACY[k]}:{v}' for k, v in sorted(legacy_used.items())))
        if unmapped:
            print('unmapped: ' + ', '.join(sorted(unmapped)))
        print('PASS' if ok else 'FAIL')
        return 0 if ok else 1
    finally:
        shutil.rmtree(work, ignore_errors=True)


if __name__ == '__main__':
    sys.exit(main())
