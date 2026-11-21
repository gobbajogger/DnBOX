#!/usr/bin/env python3
"""Stage and execute original AudioKit4.9.2 Swift tuning math, not the iOS app.

Only ObjC exposure annotations are removed from original class files; original
math bodies remain byte-for-byte. Pinned helper/type fragments and host settings
replace platform plumbing. No original-app A4 callback or audio claim is made.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess


def sha(data):
    return hashlib.sha256(data).hexdigest()


def checked_relative(name):
    p = Path(name)
    if p.is_absolute() or '..' in p.parts or not p.parts:
        raise ValueError('invalid source path')
    return p


def verify_inputs(root, manifest_bytes=None):
    if manifest_bytes is None:
        manifest_bytes = (root / 'SOURCE-MANIFEST.json').read_bytes()
    manifest = json.loads(manifest_bytes)
    files = {}
    for entry in manifest['files']:
        name = entry['path']; checked_relative(name)
        if name in files:
            raise ValueError('duplicate source path')
        data = (root / 'upstream' / name).read_bytes()
        blob = hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()
        if len(data) != entry['bytes'] or sha(data) != entry['sha256'] or blob != entry['sha']:
            raise ValueError('original source mismatch: ' + name)
        files[name] = data
    return files


def block(text, marker):
    if text.count(marker) != 1:
        raise ValueError('original fragment marker is not unique')
    start = text.index(marker); brace = text.index('{', start); depth = 0
    for i in range(brace, len(text)):
        if text[i] == '{': depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0: return text[start:i + 1]
    raise ValueError('unterminated original fragment')


def swift_source(files, harness):
    prefix = 'AudioKit/AudioKit/Common/Internals/'
    helper = files[prefix + 'AudioKitHelpers.swift'].decode()
    alias = 'public typealias MIDINoteNumber = UInt8'
    if helper.count(alias) != 1: raise ValueError('original MIDI type changed')
    support = ['import Foundation', alias, block(helper, 'extension ClosedRange {'),
               'prefix operator ❗️', block(helper, 'prefix public func ❗️(a: Bool) -> Bool {'),
               'public enum AKSettings { public static var sampleRate: Double = 48000 }',
               'func AKLog(_ values: Any...) {}']
    transformations = {}
    for name in ['AKTuningTableBase.swift', 'AKTuningTable.swift', 'AKTuningTable+EqualTemperament.swift']:
        path = prefix + 'Microtonality/' + name
        original = files[path].decode()
        # Linux Swift lacks Objective-C exposure. This does not rewrite math,
        # getters, initializers, data types, iteration order or clipping.
        transformed = original.replace('@objc ', '')
        transformations[path] = {'original_sha256': sha(files[path]),
                                 'objc_annotations_removed': original.count('@objc '),
                                 'compiled_text_sha256': sha(transformed.encode())}
        support.append(transformed)
    support.append(harness.decode())
    return ('\n\n'.join(support) + '\n').encode(), transformations


def stage(repo, out):
    if out.exists(): raise ValueError('output must be fresh')
    fixture = repo / 'tests/fixtures/synthone-tuning'
    source_manifest = (fixture / 'SOURCE-MANIFEST.json').read_bytes()
    files = verify_inputs(fixture, source_manifest)
    harness = (fixture / 'reference-main.swift').read_bytes()
    spec = importlib.util.spec_from_file_location('app_field_audit', repo / 'tools/audit-synthone-app-fields.py')
    audit = importlib.util.module_from_spec(spec); spec.loader.exec_module(audit)
    banks = repo / 'assets/factory_banks/synthone'
    captured_banks = {}
    bank_audit = audit.audit(banks, capture=captured_banks)
    manifest = json.loads(captured_banks['MANIFEST.json'])
    cases = []
    for entry in manifest['files']:
        for index, program in enumerate(json.loads(captured_banks[entry['name']])):
            cases.append({'id': entry['name'] + ':' + str(index), 'master': program.get('tuningMasterSet')})
    if len(cases) != 694: raise ValueError('expected exact694 corpus')
    generated, transformations = swift_source(files, harness)
    out.mkdir(parents=True)
    bound = {}
    payloads = {'main.swift': generated, 'cases.json': (json.dumps(cases, sort_keys=True) + '\n').encode(),
                'SOURCE-MANIFEST.json': source_manifest,
                'reference-main.swift': harness}
    payloads.update({'inputs/' + name: data for name, data in files.items()})
    payloads.update({'banks/' + name: data for name, data in captured_banks.items()})
    for name, data in payloads.items():
        p = out / name; p.parent.mkdir(parents=True, exist_ok=True); p.write_bytes(data)
        bound[name] = sha(data)
    receipt = {'scope': __doc__, 'status': 'STAGED_NOT_EXECUTED', 'inputs': bound,
               'transformations': transformations, 'cases': len(cases),
               'bank_manifest_sha256': bank_audit['manifest_sha256'],
               'runner_sha256': sha(Path(__file__).read_bytes()),
               'audit_sha256': sha((repo / 'tools/audit-synthone-app-fields.py').read_bytes())}
    (out / 'INPUTS.json').write_text(json.dumps(receipt, indent=2) + '\n')
    return receipt


def verify_stage(out, receipt):
    for name, digest in receipt['inputs'].items():
        if sha((out / checked_relative(name)).read_bytes()) != digest:
            raise ValueError('staged input changed: ' + name)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--stage-only', action='store_true')
    args = p.parse_args(); out = args.output.resolve(); repo = Path(__file__).resolve().parents[1]
    receipt = stage(repo, out); verify_stage(out, receipt)
    if args.stage_only: return
    compiler = shutil.which('swiftc')
    if not compiler:
        (out / 'RESULT.json').write_text(json.dumps({'status': 'NOT_RUN', 'reason': 'swiftc unavailable'}) + '\n')
        raise SystemExit(2)
    version = subprocess.check_output([compiler, '--version'], text=True)
    command = [compiler, '-O', str(out / 'main.swift'), '-o', str(out / 'reference')]
    (out / 'compile-command.json').write_text(json.dumps(command) + '\n')
    subprocess.run(command, check=True, cwd=out); verify_stage(out, receipt)
    outputs = {}
    for rate in [44100, 48000, 96000]:
        path = out / ('tables-' + str(rate) + '.json')
        subprocess.run([str(out / 'reference'), str(out / 'cases.json'), str(rate), str(path)], check=True, cwd=out)
        verify_stage(out, receipt)
        value = json.loads(path.read_bytes())
        if value['sample_rate'] != rate or len(value['tables']) != 694:
            raise ValueError('reference output corpus mismatch')
        for table in value['tables']:
            if len(table['float_words']) != 128 or len(table['double_words']) != 128:
                raise ValueError('reference output table length mismatch')
        outputs[path.name] = {'sha256': sha(path.read_bytes()), 'bytes': path.stat().st_size}
    (out / 'RESULT.json').write_text(json.dumps({'status': 'EXECUTED_SOURCE_MATH',
        'scope': __doc__, 'compiler_version': version, 'outputs': outputs,
        'inputs_sha256': sha((out / 'INPUTS.json').read_bytes()),
        'original_iOS_app_audio': 'NOT_RUN'}, indent=2) + '\n')


if __name__ == '__main__': main()
