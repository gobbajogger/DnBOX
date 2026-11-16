#!/usr/bin/env python3
"""Audit source factory fields outside the qualified scalar/pre-FX bench.

Counts stored inputs, not audible behavior or complete app qualification.
Expected repository/counts must come from a separately trusted manifest.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path


def finite(value):
    return isinstance(value, (int, float)) and not isinstance(value, bool) and math.isfinite(value)


def audit(bank_root):
    manifest_bytes = (bank_root / 'MANIFEST.json').read_bytes()
    manifest = json.loads(manifest_bytes)
    seen = set()
    rows = []
    counts = {'presets': 0, 'non440_A4': 0, 'non12ET_master_sets': 0,
              'missing_master_sets': 0, 'nonzero_stored_hold': 0, 'nondefault_modwheel_routing': 0}
    for entry in manifest['files']:
        name = entry['name']
        if name in seen or Path(name).name != name or name in ('.', '..'):
            raise ValueError('invalid or duplicate bank name')
        seen.add(name)
        data = (bank_root / name).read_bytes()
        blob = hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()
        if len(data) != entry['size'] or hashlib.sha256(data).hexdigest() != entry['sha256'] or blob != entry['sha']:
            raise ValueError('bank hash/size mismatch: ' + name)
        programs = json.loads(data)
        if not isinstance(programs, list) or len(programs) != entry['count']:
            raise ValueError('bank count mismatch: ' + name)
        for index, preset in enumerate(programs):
            a4, hold, wheel = (preset.get(k, default) for k, default in
                               [('frequencyA4', 440), ('isHoldMode', 0), ('modWheelRouting', 0)])
            if not all(finite(x) for x in (a4, hold, wheel)):
                raise ValueError('nonfinite app field')
            master = preset.get('tuningMasterSet')
            non12 = False
            if master is None:
                counts['missing_master_sets'] += 1
            else:
                if not isinstance(master, list) or not master or not all(finite(x) and x != 0 for x in master):
                    raise ValueError('invalid tuning master set')
                # Match original octave reduction, not tuning display-name guesses.
                reduced = []
                for frequency in master:
                    x = abs(frequency)
                    while x < 1:
                        x *= 2
                    while x >= 2:
                        x /= 2
                    reduced.append(x)
                reduced.sort()
                non12 = len(reduced) != 12 or any(abs(x - 2 ** (i / 12)) > 1e-12 for i, x in enumerate(reduced))
            flags = {'non440_A4': a4 != 440, 'non12ET_master_sets': non12,
                     'nonzero_stored_hold': hold != 0, 'nondefault_modwheel_routing': wheel != 0}
            counts['presets'] += 1
            for flag, enabled in flags.items():
                counts[flag] += int(enabled)
            if any(flags.values()):
                rows.append({'bank': name, 'index': index, 'name': preset.get('name'),
                             'frequencyA4': a4, 'isHoldMode': hold, 'modWheelRouting': wheel,
                             'tuningName': preset.get('tuningName'),
                             'tuning_master_count': len(master) if master is not None else None,
                             'flags': flags})
    return {'scope': __doc__, 'manifest_sha256': hashlib.sha256(manifest_bytes).hexdigest(),
            'source_repo': manifest['source_repo'], 'source_pin': manifest['source_pin'],
            'bank_count': len(seen), 'counts': counts, 'affected_entries': rows,
            'qualification': 'NOT_RUN: complete tuning, hold and modwheel routing'}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--banks', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    args.output.write_text(json.dumps(audit(args.banks), indent=2) + '\n')


if __name__ == '__main__':
    main()
