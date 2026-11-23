"""Compile comparison-only probes with the verified factory builder's commands."""
import argparse
import json
from pathlib import Path
import subprocess


def probe(name, commands, build, out):
    compile_cmd = next(cmd.copy() for cmd in commands
                       if any(item.endswith("/preset_audio.cpp") for item in cmd))
    source = Path(__file__).resolve().with_name(name + ".cpp")
    for flag, value in (("-c", source), ("-o", out / (name + ".o")), ("-MF", out / (name + ".d"))):
        compile_cmd[compile_cmd.index(flag) + 1] = str(value)
    subprocess.run(compile_cmd, check=True)
    link = commands[-1].copy()
    link[link.index(str(build / "factory-6.o"))] = str(out / (name + ".o"))
    link[link.index("-o") + 1] = str(out / name)
    subprocess.run(link, check=True)
    inputs = build / "factory-inputs"
    with (out / (name + ".csv")).open("w") as log:
        subprocess.run([out / name, inputs / "public-morphs.f32", inputs / "compact.bin",
                        build / "commands/preset-commands.bin"], stdout=log, check=True)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("build", type=Path)
    ap.add_argument("out", type=Path)
    args = ap.parse_args()
    build, out = args.build.resolve(), args.out.resolve()
    out.mkdir(exist_ok=False, parents=True)
    commands = json.loads((build / "factory-build-commands.json").read_text())
    for name in ("targets", "authored_histories"):
        probe(name, commands, build, out)


if __name__ == "__main__":
    main()
