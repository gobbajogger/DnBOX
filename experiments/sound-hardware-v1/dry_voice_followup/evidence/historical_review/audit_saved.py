#!/usr/bin/env python3
"""Read-only audit of saved dry-voice artifacts; never runs a renderer or metric.

Writes only its review receipt beside this script. Historical source is not read.
"""
from pathlib import Path
import ast
from collections import Counter
from datetime import datetime, timezone
import dis
import hashlib
import json
import math
import types

import numpy as np

WORK = Path('/workspace/scratch/e803955aec60')
ROOT = WORK / 's1_dry_voice_unit'
PACKET = WORK / 'dry_reference_packet'
OUT = Path(__file__).resolve().parent
METRIC = 'bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1'
MONO_BYTES = 1058304
PAIRED_BYTES = 2116608
issues = []
checked_files = {}


def check(condition, code, path, detail=''):
    if not condition:
        issues.append({'code': code, 'path': str(path), 'detail': detail})


def digest(path):
    data = path.read_bytes()
    result = hashlib.sha256(data).hexdigest()
    checked_files[str(path.relative_to(WORK))] = {'sha256': result, 'bytes': len(data)}
    return result


def read_json(path):
    digest(path)
    return json.loads(path.read_text())


def json_stream(path):
    digest(path)
    text = path.read_text()
    items = []
    decoder = json.JSONDecoder()
    while text.strip():
        text = text.lstrip()
        try:
            item, length = decoder.raw_decode(text)
        except json.JSONDecodeError:
            check(False, 'non_json_log_tail', path, text[:1000])
            break
        items.append(item)
        text = text[length:]
    return items


def accepted(row, nonfinite):
    return all(math.isfinite(row[k]) for k in ('rms_db', 'spectral_db')) and row['rms_db'] <= 1 and row['spectral_db'] <= 3 and nonfinite == 0


def assert_opcode_count(code):
    return sum(i.opname == 'LOAD_ASSERTION_ERROR' for i in dis.get_instructions(code)) + sum(assert_opcode_count(c) for c in code.co_consts if isinstance(c, types.CodeType))


plan = read_json(PACKET / 'case_plan.json')
physical = [c for c in plan['cases'] if c['case_id'].startswith('physical-gain/')]
check(len(physical) == 60 and len({c['case_id'] for c in physical}) == 60, 'physical_case_count', PACKET / 'case_plan.json')
check([n['midi_note'] for n in plan['notes']] == [36, 60, 84], 'plan_notes', PACKET / 'case_plan.json')
check(digest(PACKET / 'metrics.py') == METRIC, 'metric_hash', PACKET / 'metrics.py')
references = {}
for case in physical:
    path = PACKET / case['current_reference_mono_relative_path']
    raw = path.read_bytes()
    check(digest(path) == case['current_reference_mono_sha256'], 'reference_hash', path)
    check(len(raw) == MONO_BYTES, 'reference_size', path)
    check(bool(np.isfinite(np.frombuffer(raw, dtype='<f4')).all()), 'reference_finite', path)
    references[case['case_id']] = raw

variants = {}
for label, location, name in [('A', ROOT, 'ch4_exp_e0'), ('B', ROOT / 'variant_b', 'ch4_2x_slope4')]:
    declaration_path = location / 'DECLARATION.json'
    declaration = read_json(declaration_path)
    frozen_count = 0
    for text_path, expected in declaration['frozen_files_sha256'].items():
        path = Path(text_path)
        if path.is_relative_to(ROOT) or path.is_relative_to(PACKET):
            check(digest(path) == expected, 'frozen_hash', path)
            frozen_count += 1
        else:
            check(False, 'unexpected_frozen_scope', path)
    results = location / 'results' / name
    expected_stems = {c['case_id'].split('/', 1)[1] for c in physical}
    for suffix in ('candidate-mono.f32le', 'paired.f32', 'paired.json', 'paired.receipt.json', 'render.log', 'compare.log'):
        found = {p.name.removesuffix('.' + suffix) for p in results.glob('*.' + suffix)}
        check(found == expected_stems, 'file_coverage_' + suffix, results, json.dumps({'missing': sorted(expected_stems - found), 'extra': sorted(found - expected_stems)}))
    rows = []
    accepted_files = 0
    log_exits = Counter()
    case_records = []
    ordered = sorted(physical, key=lambda c: c['case_id'] != 'physical-gain/100_2000_.9')
    for case in ordered:
        stem = case['case_id'].split('/', 1)[1]
        candidate = results / (stem + '.candidate-mono.f32le')
        paired = results / (stem + '.paired.f32')
        report_path = results / (stem + '.paired.json')
        receipt_path = results / (stem + '.paired.receipt.json')
        report = read_json(report_path)
        receipt = read_json(receipt_path)
        candidate_raw = candidate.read_bytes()
        paired_raw = paired.read_bytes()
        candidate_sha = digest(candidate)
        paired_sha = digest(paired)
        check(len(candidate_raw) == MONO_BYTES, 'candidate_size', candidate)
        check(len(paired_raw) == PAIRED_BYTES, 'paired_size', paired)
        check(candidate_sha == receipt['candidate_sha256'], 'candidate_hash', receipt_path)
        check(paired_sha == report['pcm_sha256'], 'paired_hash', report_path)
        check(receipt['case_id'] == case['case_id'], 'receipt_case', receipt_path)
        check(receipt['reference_layout'] == 'verified-mono', 'receipt_layout', receipt_path)
        check(receipt['reference_uncompressed_sha256'] == case['current_reference_mono_sha256'], 'receipt_reference_hash', receipt_path)
        check(receipt['historical_source_paired_sha256'] == case['historical_paired_pcm_sha256'], 'receipt_historical_hash', receipt_path)
        check(receipt['metric_sha256'] == METRIC, 'receipt_metric', receipt_path)
        check(receipt['comparison_gain'] == 1 and report['gain'] == 1, 'fixed_gain', receipt_path)
        check([r['note'] for r in report['rows']] == [36, 60, 84], 'report_notes', report_path)
        words = np.frombuffer(paired_raw, dtype='<u4').reshape(3, 88192, 2)
        check(words[:, :, 0].copy().tobytes() == references[case['case_id']], 'paired_reference_column', paired)
        check(words[:, :, 1].copy().tobytes() == candidate_raw, 'paired_candidate_column', paired)
        nonfinite = int((~np.isfinite(np.frombuffer(paired_raw, dtype='<f4'))).sum())
        check(nonfinite == report['nonfinite'] == 0, 'nonfinite', report_path)
        case_accept = all(accepted(r, nonfinite) for r in report['rows'])
        check(receipt['accepted'] == case_accept, 'receipt_acceptance', receipt_path)
        accepted_files += case_accept
        for row in report['rows']:
            rows.append({'case_id': case['case_id'], 'velocity': case['velocity'], 'cutoff_hz': case['cutoff_hz'], 'resonance': case['resonance'], **row, 'nonfinite': nonfinite, 'accepted': accepted(row, nonfinite)})
        compare_path = results / (stem + '.compare.log')
        compare_items = json_stream(compare_path)
        expected_exit = 0 if case_accept else 1
        if compare_items:
            log_exits[str(compare_items[0].get('exit_status'))] += 1
        check(len(compare_items) == 3, 'compare_log_document_count', compare_path, str(len(compare_items)))
        if len(compare_items) == 3:
            check(compare_items[0]['exit_status'] == expected_exit, 'compare_log_exit', compare_path)
            check(compare_items[1] == report, 'compare_log_report', compare_path)
            check(compare_items[2] == receipt, 'compare_log_receipt', compare_path)
        render_path = results / (stem + '.render.log')
        render_items = json_stream(render_path)
        check(len(render_items) == 2, 'render_log_document_count', render_path, str(len(render_items)))
        if len(render_items) == 2:
            check(render_items[0]['exit_status'] == 0, 'render_exit', render_path)
            expected_command = [str(location / 'build/s1_dry_host'), str(candidate), str(case['velocity']), repr(case['cutoff_hz']), repr(case['resonance']), repr(case['filter_depth'])]
            check(render_items[0]['command'] == expected_command, 'render_command', render_path)
            check(render_items[1] == {'frames_per_note': 88192, 'notes': [36, 60, 84], 'warmup_frames': 176448, 'off_frame': 66112, 'nonfinite': 0, 'phase_at_note_on': 0, 'comparison_gain': 1}, 'render_metadata', render_path)
        case_records.append({'case_id': case['case_id'], 'accepted_notes': sum(accepted(r, nonfinite) for r in report['rows']), 'accepted_file': case_accept, 'candidate_sha256': candidate_sha, 'paired_sha256': paired_sha, 'report_sha256': checked_files[str(report_path.relative_to(WORK))]['sha256'], 'receipt_sha256': checked_files[str(receipt_path.relative_to(WORK))]['sha256']})
    summary_path = results / 'all180_summary.json'
    summary = read_json(summary_path)
    check(summary['rows'] == rows, 'summary_rows', summary_path)
    totals = {'case_file_count': 60, 'note_case_count': len(rows), 'accepted_note_cases': sum(r['accepted'] for r in rows), 'accepted_case_files': accepted_files, 'nonfinite': 0, 'max_rms_db': max(r['rms_db'] for r in rows), 'max_spectral_db': max(r['spectral_db'] for r in rows)}
    for key, value in totals.items():
        check(summary[key] == value, 'summary_' + key, summary_path)
    check(len({(r['case_id'], r['note']) for r in rows}) == 180, 'unique_note_coverage', summary_path)
    zero = [r for r in rows if r['resonance'] == 0]
    selftest = read_json(location / 'results/selftest.json')
    variants[label] = {'totals': totals, 'resonance_zero': {'notes': len(zero), 'accepted': sum(r['accepted'] for r in zero)}, 'frozen_files_checked': frozen_count, 'compare_log_exits': dict(log_exits), 'selftest_saved_status': selftest['status'], 'case_records': case_records}

bundle = read_json(ROOT / 'BUNDLE-MANIFEST.json')
bundle_checked = 0
bundle_skipped = []
for item in bundle['files']:
    path = WORK / item['archive_path']
    if path.is_relative_to(ROOT) or path.is_relative_to(PACKET):
        check(digest(path) == item['sha256'], 'bundle_manifest_hash', path)
        check(path.stat().st_size == item['bytes'], 'bundle_manifest_size', path)
        bundle_checked += 1
    else:
        bundle_skipped.append(item['archive_path'])

assertions = {}
for path in (ROOT / 'run_screen.py', ROOT / 'variant_b/run_screen.py', ROOT / 'summarize.py'):
    source = path.read_text()
    digest(path)
    assertions[str(path.relative_to(WORK))] = {'ast_assert_count': sum(isinstance(n, ast.Assert) for n in ast.walk(ast.parse(source))), 'normal_assert_opcode_count': assert_opcode_count(compile(source, str(path), 'exec', optimize=0)), 'optimized_assert_opcode_count': assert_opcode_count(compile(source, str(path), 'exec', optimize=1))}

symlinks = [str(p.relative_to(WORK)) for base in (ROOT, PACKET) for p in base.rglob('*') if p.is_symlink()]
receipt = {'schema': 'independent dry saved-artifact audit v1', 'reviewed_utc': datetime.now(timezone.utc).isoformat(), 'status': 'PASS_SAVED_ARTIFACT_CONSISTENCY_ONLY' if not issues else 'SAVED_ARTIFACT_FINDINGS', 'audio_gate': 'FAIL_BOTH_VARIANTS', 'gate_admission': False, 'method': 'Read original allowed candidate/harness files; inspect and hash saved PCM/report/receipt/logs; compare interleaved PCM columns bit-for-bit; no metric execution or new audio render.', 'scope_limits': ['No original S1/SJ6 implementation or tables read', 'No source-aware candidate read', 'No target build/native/emulator/full-suite execution', 'No original runner execution', 'Saved numerical RMS/spectral values were reconciled, not recomputed'], 'reference_files_checked': len(references), 'bundle_manifest_files_checked': bundle_checked, 'bundle_manifest_files_outside_scope_not_read': bundle_skipped, 'variants': variants, 'optimized_python_assertion_analysis': assertions, 'symlinks_in_reviewed_input_trees': symlinks, 'issues': issues, 'checked_files': checked_files}
destination = OUT / 'machine_receipt.json'
with destination.open('x') as output:
    json.dump(receipt, output, indent=2)
    output.write('\n')
print(json.dumps({'receipt': str(destination), 'status': receipt['status'], 'reference_files_checked': len(references), 'bundle_manifest_files_checked': bundle_checked, 'bundle_manifest_files_outside_scope_not_read': bundle_skipped, 'variants': {k: {kk: vv for kk, vv in v.items() if kk != 'case_records'} for k, v in variants.items()}, 'optimized_python_assertion_analysis': assertions, 'symlinks': symlinks, 'issues': issues}, indent=2))
