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
    commands = json.loads((base/'commands.json').read_text())
    replacements = {}
    planned = []
    for index, original in enumerate(commands[:-1]):
        command = list(original)
        if '-c' not in command or '-o' not in command:
            raise ValueError('Expected compile command')
        old_object = command[command.index('-o') + 1]
        new_object = str(out/f'dependency-{index:03}.o')
        replacements[old_object] = new_object
        command[command.index('-o') + 1] = new_object
        source_index = command.index('-c') + 1
        if command[source_index].endswith('/tests/source_scheduler_test.cpp'):
            command[source_index] = str(repo/'tests/host/synthone_renderer_test.cpp')
            command += ['-I' + str(repo/'firmware/src/dsp')]
        planned.append(command)
    link = [replacements.get(value, value) for value in commands[-1]]
    if any(value.endswith('.o') and value not in replacements.values() for value in link):
        raise ValueError('Link contains an object without a fresh compile command')
    link[link.index('-o') + 1] = str(out/'adapter-test')
    planned.append(link)
    (out/'commands.json').write_text(json.dumps(planned, indent=2) + '\n')
    for command in planned:
        subprocess.run(command, check=True)
    subprocess.run([str(out/'adapter-test'), str(base/'public-morphs.f32'),
                    str(base/'compact.bin'), str(out/'adapter-audio.f32le')], check=True)


if __name__ == '__main__':
    main()
