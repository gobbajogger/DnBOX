#!/usr/bin/env python3
"""Freshly rebuild the original-source pre-FX A/B adapter and dependencies.

Requires a trusted pinned direct-scheduler source-build closure and commands.
No retained .o is reused. This remains a host/component test, not a factory,
full-FX, device xrun, or all-engine memory qualification.
"""
import argparse
import hashlib
import json
import pathlib
import subprocess
import shutil
import shlex


def stage_runtime(closure, runtime):
    # Validate the complete overlay before replacing any retained fixture file.
    for path in runtime.rglob('*'):
        relative = path.relative_to(runtime)
        if any(part in ('tests', 'oracle', 'sequence-upstream') for part in relative.parts) or path.name.startswith('full_oracle'):
            raise ValueError('Protected oracle/test path: ' + str(relative))
    for group in ('scheduler', 'experiment', 'controls'):
        shutil.copytree(runtime/group, closure/group, dirs_exist_ok=True)


ADAPTER_FILES = ('tests/host/synthone_renderer_test.cpp',
                 'firmware/src/dsp/synthone_renderer.hpp',
                 'firmware/src/dsp/synthone_c.cpp', 'firmware/src/dsp/synthone_c.h',
                 'tests/fixtures/synthone-factory/frontend/oracle_controls.cpp',
                 'tests/fixtures/synthone-factory/frontend/oracle_controls.hpp',
                 'tests/fixtures/synthone-factory/frontend/S1Rate.hpp',
                 'tests/fixtures/synthone-factory/frontend/AKSynthOneRate.h',
                 'tests/fixtures/synthone-factory/fullfx/compat/AudioKit/AKInterop.h')


def stage_adapters(repo, destination):
    files = ADAPTER_FILES
    hashes = {}
    for name in files:
        data = (repo/name).read_bytes()
        path = destination/name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        hashes[name] = hashlib.sha256(data).hexdigest()
    return hashes


def verify_bound(allowed):
    for path, digest in allowed.items():
        if hashlib.sha256(path.read_bytes()).hexdigest() != digest:
            raise ValueError('Compiled input changed: ' + str(path))


def verify_depfile(depfile, allowed):
    value = depfile.read_text().replace('\\\n', ' ')
    for name in shlex.split(value.split(':', 1)[1]):
        path = pathlib.Path(name).resolve()
        if path not in allowed:
            raise ValueError('Unverified actual compile dependency: ' + str(path))
        if hashlib.sha256(path.read_bytes()).hexdigest() != allowed[path]:
            raise ValueError('Changed actual compile dependency: ' + str(path))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-build', type=pathlib.Path, required=True)
    parser.add_argument('--output', type=pathlib.Path, required=True)
    args = parser.parse_args()
    base, out = args.source_build.resolve(), args.output.resolve()
    if base == out:
        raise ValueError('Output must not overwrite source-build objects')
    out.mkdir(parents=True, exist_ok=True)
    repo = pathlib.Path(__file__).resolve().parents[2]
    for entry in json.loads((base/'COMPILED-SOURCES.json').read_text()):
        path = base/'closure/scheduler'/entry['path']
        if hashlib.sha256(path.read_bytes()).hexdigest() != entry['sha256']:
            raise ValueError('Source hash mismatch: ' + entry['path'])
    closure = (base/'closure').resolve()
    closure_hashes = json.loads((base/'CLOSURE-SHA256.json').read_text())
    verified = set()
    for relative, digest in closure_hashes.items():
        path = (closure/relative).resolve()
        if not path.is_relative_to(closure) or not path.is_file():
            raise ValueError('Invalid closure path: ' + relative)
        if hashlib.sha256(path.read_bytes()).hexdigest() != digest:
            raise ValueError('Closure hash mismatch: ' + relative)
        verified.add(path)
    actual = {path.resolve() for path in closure.rglob('*') if path.is_file()}
    if actual != verified:
        raise ValueError('Closure contains unverified or missing files')
    # The verified retained closure supplies oracle/fixture inputs. Always
    # compile the current repository's runtime, not historical runtime bytes.
    original_closure = closure
    closure = out/'closure'
    if closure.exists():
        raise ValueError('Use a fresh output directory for runtime staging')
    shutil.copytree(original_closure, closure)
    runtime = repo/'firmware/src/dsp/synthone'
    stage_runtime(closure, runtime)
    verified = {path.resolve() for path in closure.rglob('*') if path.is_file()}
    tested = {str(path.relative_to(closure)): hashlib.sha256(path.read_bytes()).hexdigest()
              for path in sorted(verified)}
    (out/'TESTED-SOURCES.json').write_text(json.dumps(tested, indent=2) + '\n')
    adapters = out/'adapter-inputs'
    adapter_hashes = stage_adapters(repo, adapters)
    (out/'ADAPTER-SOURCES.json').write_text(json.dumps(adapter_hashes, indent=2) + '\n')
    allowed = {path.resolve(): digest for name, digest in tested.items() for path in [closure/name]}
    allowed.update({(adapters/name).resolve(): digest for name, digest in adapter_hashes.items()})
    oracle_flags = ['-I' + str(adapters/'tests/fixtures/synthone-factory/frontend'),
                    '-I' + str(adapters/'tests/fixtures/synthone-factory/fullfx/compat'),
                    '-Wno-error=deprecated']
    commands = json.loads((base/'commands.json').read_text())
    commands = [[value.replace(str(original_closure), str(closure)) for value in command]
                for command in commands]
    replacements = {}
    planned = []
    adapter_count = 0
    for index, original in enumerate(commands[:-1]):
        command = list(original)
        if '-c' not in command or '-o' not in command:
            raise ValueError('Expected compile command')
        old_object = command[command.index('-o') + 1]
        new_object = str(out/f'dependency-{index:03}.o')
        replacements[old_object] = new_object
        command[command.index('-o') + 1] = new_object
        source_index = command.index('-c') + 1
        source = pathlib.Path(command[source_index]).resolve()
        if source not in verified:
            raise ValueError('Unverified compile source: ' + str(source))
        for flag in command:
            if flag.startswith('-I'):
                include = pathlib.Path(flag[2:]).resolve()
                if not include.is_relative_to(closure) or not include.is_dir():
                    raise ValueError('Unverified include directory: ' + flag)
            elif flag.startswith(('-include', '-imacros', '-isystem', '-iquote', '@')):
                raise ValueError('Unsupported implicit input flag: ' + flag)
        if source == closure/'scheduler/tests/source_scheduler_test.cpp':
            adapter_count += 1
            wrapper_compile = list(command)
            command[source_index] = str(adapters/'tests/host/synthone_renderer_test.cpp')
            command += ['-I' + str(adapters/'firmware/src/dsp')] + oracle_flags
        planned.append(command)
    if adapter_count != 1:
        raise ValueError('Expected exactly one adapter harness')
    wrapper_compile[wrapper_compile.index('-c') + 1] = str(adapters/'firmware/src/dsp/synthone_c.cpp')
    wrapper_compile[wrapper_compile.index('-o') + 1] = str(out/'c-wrapper.o')
    wrapper_compile += ['-I' + str(adapters/'firmware/src/dsp')]
    planned.append(wrapper_compile)
    frontend_compile = list(wrapper_compile)
    frontend_compile[frontend_compile.index('-c') + 1] = str(closure/'controls/frontend/source_controls.cpp')
    frontend_compile[frontend_compile.index('-o') + 1] = str(out/'source-frontend.o')
    planned.append(frontend_compile)
    oracle_compile = list(wrapper_compile)
    oracle_compile[oracle_compile.index('-c') + 1] = str(adapters/'tests/fixtures/synthone-factory/frontend/oracle_controls.cpp')
    oracle_compile[oracle_compile.index('-o') + 1] = str(out/'oracle-frontend.o')
    # Retain original source warning; invalid getter IDs are guarded by wrapper.
    oracle_compile += oracle_flags + ['-Wno-error=maybe-uninitialized']
    planned.append(oracle_compile)
    link = [replacements.get(value, value) for value in commands[-1]]
    if any(value.endswith('.o') and value not in replacements.values() for value in link):
        raise ValueError('Link contains an object without a fresh compile command')
    link.insert(1, str(out/'c-wrapper.o'))
    link.insert(1, str(out/'source-frontend.o'))
    link.insert(1, str(out/'oracle-frontend.o'))
    link[link.index('-o') + 1] = str(out/'adapter-test')
    planned.append(link)
    (out/'commands.json').write_text(json.dumps(planned, indent=2) + '\n')
    for index, command in enumerate(planned):
        verify_bound(allowed)
        if '-c' in command:
            depfile = out/f'compile-{index:03}.d'
            command += ['-MMD', '-MF', str(depfile)]
            subprocess.run(command, check=True)
            verify_depfile(depfile, allowed)
        else:
            subprocess.run(command, check=True)
        verify_bound(allowed)
    (out/'commands.json').write_text(json.dumps(planned, indent=2) + '\n')
    subprocess.run([str(out/'adapter-test'), str(base/'public-morphs.f32'),
                    str(base/'compact.bin'), str(out/'adapter-audio.f32le')], check=True)
    verify_bound(allowed)


if __name__ == '__main__':
    main()
