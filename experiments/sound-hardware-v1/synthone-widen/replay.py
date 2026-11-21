#!/usr/bin/env python3
"""Run a bounded widening delay against the original full factory graph at 44.1 kHz."""
import argparse
import hashlib
import json
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent / "sj6-factory"))
from compare import channel_score


def replace_once(text, old, new):
    if text.count(old) != 1:
        raise ValueError(f"fixture source changed at {old}")
    return text.replace(old, new)


def build_fixture(build_dir, output, bits):
    commands = json.loads((build_dir / "factory-build-commands.json").read_text())
    compile_cmd = next(c.copy() for c in commands if "-c" in c and
                       c[c.index("-c") + 1].endswith("/preset_audio.cpp"))
    source = Path(compile_cmd[compile_cmd.index("-c") + 1]).read_text()
    source = replace_once(source, "struct Context{",
                          'extern "C" void use_bounded_widen(sp_delay *);\nstruct Context{')
    source = replace_once(source, "SourceFx a(48000),b(48000);",
                          "SourceFx a(44100),b(44100);use_bounded_widen(a.widenDelay);")
    source = replace_once(source, "FullPair p(fixture,48000,", "FullPair p(fixture,44100,")
    source = source.replace("96000", "88200").replace("12000", "11025").replace("72000", "66150")
    source = replace_once(source, "CHECK(!memcmp(sl,ol,n*4)&&!memcmp(sr,orr,n*4))",
                          "CHECK(!memcmp(sl,ol,n*4))")
    source = source.replace("paired_bitexact", "left_bitexact")
    (output / "pair.cpp").write_text(source)
    src_obj = compile_cmd[compile_cmd.index("-o") + 1]
    for flag, name in [("-c", "pair.cpp"), ("-o", "pair.o"), ("-MF", "pair.d")]:
        compile_cmd[compile_cmd.index(flag) + 1] = str(output / name)
    subprocess.run(compile_cmd, check=True)
    inc = build_dir / "closure/experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe"
    subprocess.run(["cc", "-std=gnu11", "-O2", "-DNO_LIBSNDFILE", f"-DTEST_HISTORY_BITS={bits}", "-I" + str(inc),
                    "-I" + str(inc / "lib/dr_wav"), "-c", HERE / "fixture.c",
                    "-o", output / "fixture.o"], check=True)
    link = [str(output / "pair.o") if v == src_obj else v for v in commands[-1]]
    link[link.index("-o") + 1] = str(output / "pair")
    link[1:1] = [str(output / "fixture.o"), "-Wl,--wrap=sp_delay_compute"]
    subprocess.run(link, check=True)


def commands(path):
    data = path.read_bytes()
    magic, count = struct.unpack_from("<II", data)
    if (magic, count) != (0x53315043, 694):
        raise ValueError("wrong factory command stream")
    result = []
    pos = 8
    for i in range(count):
        n, = struct.unpack_from("<I", data, pos)
        end = pos + 4 + n * 8
        if n != 144 or end > len(data):
            raise ValueError(f"truncated program {i}")
        result.append(data[pos:end])
        pos = end
    if pos != len(data):
        raise ValueError("unconsumed factory data")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--indices", nargs="+", type=int)
    parser.add_argument("--history-bits", type=int, choices=(16, 24), default=24)
    args = parser.parse_args()
    root, out = args.build.resolve(), args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    build_fixture(root, out, args.history_bits)
    stream = commands(root / "commands/preset-commands.bin")
    indices = list(range(694)) if args.indices is None else args.indices
    if len(set(indices)) != len(indices) or not all(0 <= i < len(stream) for i in indices):
        raise ValueError("invalid program selection")
    assets = root / "factory-inputs"
    with (out / "results.jsonl").open("w") as records:
        for start in range(0, len(indices), 4):
            batch = indices[start:start + 4]
            with tempfile.TemporaryDirectory(dir=out) as tmp:
                tmp = Path(tmp)
                inputs = tmp / "commands.bin"
                inputs.write_bytes(struct.pack("<II", 0x53315043, len(batch)) +
                                   b"".join(stream[i] for i in batch))
                with (out / "pair.log").open("a") as log:
                    subprocess.run([out / "pair", assets / "public-morphs.f32", assets / "compact.bin",
                                    inputs, tmp], check=True, stdout=log, stderr=subprocess.STDOUT)
                for offset, index in enumerate(batch):
                    pair = [(tmp / f"preset-{offset:02}-{arm}.f32le.pcm").read_bytes()
                            for arm in ("original", "compact")]
                    if any(len(data) != 88200 * 8 for data in pair):
                        raise ValueError(f"incomplete capture for {index}")
                    a, b = [np.frombuffer(data, "<f4").reshape(-1, 2) for data in pair]
                    if not np.isfinite(a).all() or not np.isfinite(b).all() or not np.array_equal(a[:,0], b[:,0]):
                        raise ValueError(f"bad left/finite control for {index}")
                    level, spectral = channel_score(a[:,1], b[:,1], 44100, [20,16000])
                    ok = level is not None and abs(level) <= 1 and spectral is not None and spectral <= 3
                    row = dict(index=index, frames=len(a), original=hashlib.sha256(pair[0]).hexdigest(),
                               bounded=hashlib.sha256(pair[1]).hexdigest(),
                               right_changed_words=int(np.count_nonzero(a[:,1]!=b[:,1])),
                               right_level_db=level, right_spectral_rms_db=spectral, passed=ok)
                    print(json.dumps(row, allow_nan=False), file=records, flush=True)
            print(f"{start + len(batch)}/{len(indices)}", flush=True)
    rows = [json.loads(line) for line in (out / "results.jsonl").read_text().splitlines()]
    return not all(row["passed"] for row in rows)


if __name__ == "__main__":
    sys.exit(main())
