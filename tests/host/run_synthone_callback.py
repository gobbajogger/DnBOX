#!/usr/bin/env python3
"""Fresh source-bound host callback deadlines; no device XRUN/app claim."""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import platform
import subprocess
import sys


def sha(data):
    return hashlib.sha256(data).hexdigest()


def verify_tool_snapshot(snapshot):
    for path, data in snapshot.items():
        if path.read_bytes() != data:
            raise ValueError('orchestration tool changed: ' + str(path))


def parse_benchmark_tracer(stderr):
    lines = [line for line in stderr.splitlines() if line.startswith('benchmark_tracer_pid=')]
    if len(lines) != 1:
        raise ValueError('missing or duplicate benchmark tracer observation')
    value = int(lines[0].split('=', 1)[1].split()[0])
    if value < -1: raise ValueError('invalid benchmark tracer observation')
    return value


def summarize(csv_path, programs):
    totals = {'callbacks': 0, 'frames': 0, 'wall_misses': 0, 'cpu_misses': 0,
              'max_wall_ns': 0, 'max_cpu_ns': 0}
    with csv_path.open() as stream:
        for index, row in enumerate(csv.DictReader(stream)):
            if int(row['preset']) != index // 750 or int(row['block']) != index % 750:
                raise ValueError('callback trace identity/order mismatch')
            frames, rate = int(row['frames']), int(row['rate'])
            if (frames, rate) != (128, 48000): raise ValueError('deadline changed')
            totals['callbacks'] += 1
            totals['frames'] += frames
            for clock in ('wall', 'cpu'):
                value = int(row[clock + '_ns'])
                if value < 0: raise ValueError('negative elapsed time')
                miss = value * rate > frames * 1000000000
                if int(row[clock + '_miss']) != int(miss): raise ValueError('deadline count mismatch')
                totals[clock + '_misses'] += int(miss)
                totals['max_' + clock + '_ns'] = max(totals['max_' + clock + '_ns'], value)
    if totals['callbacks'] != programs * 750:
        raise ValueError('incomplete callback trace')
    return totals


def summarize_resources(resources_path, trace_path, programs):
    import itertools
    fields = ('minor_faults', 'major_faults', 'voluntary_switches', 'involuntary_switches', 'user_ns', 'system_ns')
    totals = {key: 0 for key in fields}
    summary = {'callbacks': 0, 'totals': totals, 'cpu_migrations': 0, 'unknown_cpu_observations': 0,
               'miss_rows': 0, 'miss_rows_with_faults': 0, 'miss_rows_with_switches': 0,
               'scope': 'Resource bracket includes clock probes around timed region; association is not causation'}
    with resources_path.open() as rf, trace_path.open() as tf:
        for index, pair in enumerate(itertools.zip_longest(csv.DictReader(rf), csv.DictReader(tf))):
            resource, trace = pair
            if resource is None or trace is None: raise ValueError('resource/trace row count mismatch')
            identity = (index // 750, index % 750)
            if any((int(row['preset']), int(row['block'])) != identity for row in pair):
                raise ValueError('resource/trace identity mismatch')
            values = {key: int(resource[key]) for key in fields}
            if any(value < 0 for value in values.values()): raise ValueError('negative resource delta')
            for key in fields: totals[key] += values[key]
            before, after = int(resource['cpu_before']), int(resource['cpu_after'])
            summary['unknown_cpu_observations'] += int(before < 0 or after < 0)
            summary['cpu_migrations'] += int(before >= 0 and after >= 0 and before != after)
            miss = bool(int(trace['wall_miss']) or int(trace['cpu_miss']))
            summary['miss_rows'] += int(miss)
            summary['miss_rows_with_faults'] += int(miss and bool(values['minor_faults'] or values['major_faults']))
            summary['miss_rows_with_switches'] += int(miss and bool(values['voluntary_switches'] or values['involuntary_switches']))
            summary['callbacks'] += 1
    if summary['callbacks'] != programs * 750: raise ValueError('incomplete resource trace')
    return summary


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source-build', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--programs', type=int, default=694)
    p.add_argument('--build-only', action='store_true')
    p.add_argument('--diagnostic-resources', action='store_true', help='Instrument resource counters outside timed interval; never qualification')
    p.add_argument('--diagnostic-ftz', action='store_true', help='Host-only denormal diagnostic; never qualification')
    args = p.parse_args()
    if not 1 <= args.programs <= 694: raise ValueError('invalid program count')
    repo, out = Path(__file__).resolve().parents[2], args.output.resolve()
    # Snapshot new benchmark before starting any compilation.
    runner_bytes = Path(__file__).read_bytes()
    benchmark = (repo / 'tests/fixtures/synthone-callback/bench.cpp').read_bytes()
    contract = (repo / 'tests/fixtures/synthone-callback/CONTRACT.md').read_bytes()
    tool_snapshot = {repo / 'tests/host' / name: (repo / 'tests/host' / name).read_bytes()
                     for name in ('run_synthone_factory.py', 'run_synthone_renderer.py')}
    verify_tool_snapshot(tool_snapshot)
    subprocess.run([sys.executable, repo / 'tests/host/run_synthone_factory.py', '--source-build',
                    args.source_build.resolve(), '--output', out, '--build-only'], check=True)
    verify_tool_snapshot(tool_snapshot)
    import types
    factory = types.ModuleType('factory')
    factory.__file__ = str(repo / 'tests/host/run_synthone_factory.py')
    exec(compile(tool_snapshot[Path(factory.__file__)], factory.__file__, 'exec'), factory.__dict__)
    allowed = {}
    for manifest, root in [('TESTED-SOURCES.json', 'closure'), ('ADAPTER-SOURCES.json', 'adapter-inputs')]:
        for name, digest in json.loads((out / manifest).read_bytes()).items():
            allowed[(out / root / factory.relative(name)).resolve()] = digest
    for name, digest in json.loads((out / 'FACTORY-INPUTS.json').read_bytes()).items():
        allowed[(out / factory.relative(name)).resolve()] = digest
    for path, data in tool_snapshot.items():
        staged_tool = out / ('orchestration-' + path.name)
        staged_tool.write_bytes(data)
        allowed[staged_tool] = sha(data)
    source, contract_path = out / 'callback-bench.cpp', out / 'CALLBACK-CONTRACT.md'
    source.write_bytes(benchmark); contract_path.write_bytes(contract)
    (out / 'callback-runner.py').write_bytes(runner_bytes)
    allowed[(out / 'callback-runner.py').resolve()] = sha(runner_bytes)
    allowed[source] = sha(benchmark); allowed[contract_path] = sha(contract)
    commands = json.loads((out / 'factory-build-commands.json').read_bytes())
    originals = [c for c in commands if '-c' in c and c[c.index('-c') + 1].endswith('/preset_audio.cpp')]
    if len(originals) != 1: raise ValueError('expected one factory harness compile')
    compile_command = list(originals[0]); old_obj = compile_command[compile_command.index('-o') + 1]
    obj = str(out / 'callback-bench.o'); dep = out / 'callback-bench.d'
    if args.diagnostic_resources: compile_command.insert(1, '-DDNBOX_CALLBACK_RUSAGE=1')
    compile_command[compile_command.index('-c') + 1] = str(source)
    compile_command[compile_command.index('-o') + 1] = obj
    compile_command[compile_command.index('-MF') + 1] = str(dep)
    link_command = [obj if x == old_obj else x for x in commands[-1]]
    if link_command.count(obj) != 1: raise ValueError('expected one callback harness link')
    link_command[link_command.index('-o') + 1] = str(out / 'callback-bench')
    factory.verify_files(allowed)
    subprocess.run(compile_command, check=True)
    factory.verify_dependencies(dep, allowed); factory.verify_files(allowed)
    subprocess.run(link_command, check=True); factory.verify_files(allowed)
    (out / 'CALLBACK-COMMANDS.json').write_text(json.dumps([compile_command, link_command], indent=2) + '\n')
    stream = out / 'commands/preset-commands.bin'
    stream_hash = sha(stream.read_bytes())
    allowed[stream] = stream_hash
    executable_hash = sha((out / 'callback-bench').read_bytes())
    inputs = {str(path.relative_to(out)): digest for path, digest in sorted(allowed.items())}
    (out / 'CALLBACK-INPUTS.json').write_text(json.dumps(inputs, indent=2) + '\n')
    verify_tool_snapshot(tool_snapshot)
    if args.build_only:
        (out / 'CALLBACK-RESULT.json').write_text(json.dumps({'status': 'NOT_RUN', 'reason': 'build only'}) + '\n')
        return
    status = Path('/proc/self/status').read_text() if Path('/proc/self/status').exists() else 'unavailable'
    env = {'platform': platform.platform(), 'cpu_count': os.cpu_count(),
           'affinity': sorted(os.sched_getaffinity(0)) if hasattr(os, 'sched_getaffinity') else None,
           'runner_tracer_pid': next((line.split(':', 1)[1].strip() for line in status.splitlines() if line.startswith('TracerPid:')), 'unknown')}
    (out / 'CALLBACK-ENVIRONMENT.json').write_text(json.dumps(env, indent=2) + '\n')
    run_command = [str(out / 'callback-bench'), str(out / 'factory-inputs/public-morphs.f32'),
        str(out / 'factory-inputs/compact.bin'), str(stream), str(out / 'callback-trace.csv'),
        str(out / 'callback-programs.jsonl'), str(args.programs), '1' if args.diagnostic_ftz else '0']
    if args.diagnostic_resources: run_command.append(str(out / 'callback-resources.csv'))
    run = subprocess.run(run_command, text=True, capture_output=True)
    (out / 'callback-run.log').write_text(run.stdout + run.stderr)
    factory.verify_files(allowed)
    verify_tool_snapshot(tool_snapshot)
    if sha((out / 'callback-bench').read_bytes()) != executable_hash:
        raise ValueError('benchmark executable changed')
    benchmark_tracer_pid = parse_benchmark_tracer(run.stderr)
    summary = summarize(out / 'callback-trace.csv', args.programs)
    programs = [json.loads(line) for line in (out / 'callback-programs.jsonl').read_text().splitlines()]
    if len(programs) != args.programs or [x['preset'] for x in programs] != list(range(args.programs)):
        raise ValueError('incomplete program result')
    for clock in ('wall', 'cpu'):
        if sum(x[clock + '_misses'] for x in programs) != summary[clock + '_misses']:
            raise ValueError('program/trace miss counts disagree')
        if max(x['max_' + clock + '_ns'] for x in programs) != summary['max_' + clock + '_ns']:
            raise ValueError('program/trace maxima disagree')
    nonfinite = sum(x['nonfinite_words'] for x in programs)
    expected_exit = int(bool(summary['wall_misses'] or summary['cpu_misses'] or nonfinite))
    if run.returncode != expected_exit: raise ValueError('unexpected benchmark exit')
    resource_summary = None
    if args.diagnostic_resources:
        resource_summary = summarize_resources(out / 'callback-resources.csv', out / 'callback-trace.csv', args.programs)
    receipt = {'scope': __doc__, 'programs': args.programs, 'corpus_complete': args.programs == 694, 'diagnostic_ftz': args.diagnostic_ftz,
        'benchmark_tracer_pid': benchmark_tracer_pid, 'diagnostic_resources': resource_summary,
        'qualification': 'NOT_QUALIFIED: diagnostic instrumentation/mode' if (args.diagnostic_ftz or args.diagnostic_resources) else 'host measurement only',
        'summary': summary, 'nonfinite_words': nonfinite, 'process_exit': run.returncode,
        'status': 'DEADLINE_FAIL' if expected_exit else 'ZERO_MISSES_IN_THIS_RUN',
        'executable_sha256': executable_hash, 'stream_sha256': stream_hash, 'inputs_sha256': sha((out / 'CALLBACK-INPUTS.json').read_bytes()),
        'files': {name: sha((out / name).read_bytes()) for name in
                  ['callback-trace.csv', 'callback-programs.jsonl', 'CALLBACK-COMMANDS.json', 'CALLBACK-ENVIRONMENT.json', 'callback-run.log']}}
    if args.diagnostic_resources:
        receipt['files']['callback-resources.csv'] = sha((out / 'callback-resources.csv').read_bytes())
    (out / 'CALLBACK-RESULT.json').write_text(json.dumps(receipt, indent=2) + '\n')
    print(json.dumps(receipt))
    raise SystemExit(expected_exit)


if __name__ == '__main__': main()
