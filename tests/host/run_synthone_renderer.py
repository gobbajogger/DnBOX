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
    for group in ('scheduler', 'experiment', 'controls'):
        shutil.copytree(runtime/group, closure/group, dirs_exist_ok=True)
    verified = {path.resolve() for path in closure.rglob('*') if path.is_file()}
    tested = {str(path.relative_to(closure)): hashlib.sha256(path.read_bytes()).hexdigest()
              for path in sorted(verified)}
    (out/'TESTED-SOURCES.json').write_text(json.dumps(tested, indent=2) + '\n')
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
            command[source_index] = str(repo/'tests/host/synthone_renderer_test.cpp')
            command += ['-I' + str(repo/'firmware/src/dsp')]
        planned.append(command)
    if adapter_count != 1:
        raise ValueError('Expected exactly one adapter harness')
    wrapper_compile[wrapper_compile.index('-c') + 1] = str(repo/'firmware/src/dsp/synthone_c.cpp')
    wrapper_compile[wrapper_compile.index('-o') + 1] = str(out/'c-wrapper.o')
    wrapper_compile += ['-I' + str(repo/'firmware/src/dsp')]
    planned.append(wrapper_compile)
    link = [replacements.get(value, value) for value in commands[-1]]
    if any(value.endswith('.o') and value not in replacements.values() for value in link):
        raise ValueError('Link contains an object without a fresh compile command')
    link.insert(1, str(out/'c-wrapper.o'))
    link[link.index('-o') + 1] = str(out/'adapter-test')
    planned.append(link)
    (out/'commands.json').write_text(json.dumps(planned, indent=2) + '\n')
    for command in planned:
        subprocess.run(command, check=True)
    subprocess.run([str(out/'adapter-test'), str(base/'public-morphs.f32'),
                    str(base/'compact.bin'), str(out/'adapter-audio.f32le')], check=True)


if __name__ == '__main__':
    main()
