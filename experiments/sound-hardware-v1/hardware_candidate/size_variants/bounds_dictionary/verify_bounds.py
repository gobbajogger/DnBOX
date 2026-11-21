#!/usr/bin/env python3
"""Focused physical-range equivalence and target-object check, never firmware fit."""
from pathlib import Path
import argparse
import ctypes as C
import datetime
import hashlib
import json
import math
import re
import struct
import subprocess

ROOT = Path(__file__).resolve().parent
OUT = None
TC = None
REFERENCE = ROOT.parents[2] / "candidate_review/compact_profile_review/snapshot"
BOUNDS = [(0, 16), (0, 16), (0, 16), (0, 1), (0, 16), (0, 16)] * 2 + [
    (0, 1), (.03, .5), (0, 4), (0, .5), (20, 20000), (0, 1),
    (-20000, 20000), (0, 16), (0, .47), (-48, 48), (-1, 1)]


def require(condition, detail):
    if not condition:
        raise AssertionError(detail)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def bits(value):
    return struct.unpack("<I", struct.pack("<f", value))[0]


def value(raw):
    return struct.unpack("<f", struct.pack("<I", raw))[0]


def neighbor(raw, up):
    v = value(raw)
    if v == 0:
        return 1 if up else 0x80000001
    return raw + (1 if (v > 0) == up else -1)


def samples(lo, hi):
    a, b = bits(lo), bits(hi)
    return list(dict.fromkeys([
        a, neighbor(a, True), neighbor(a, False),
        b, neighbor(b, False), neighbor(b, True), bits((lo + hi) / 2),
        0, 0x80000000, 1, 0x80000001, 0x007fffff, 0x807fffff,
        0x00800000, 0x80800000, 0x7f7fffff, 0xff7fffff,
        0x7f800000, 0xff800000, 0x7fc00001, 0xffc00001,
        0x7f800001, 0xff800001]))


def load(path):
    lib = C.CDLL(str(path))
    lib.dnbx_part_size.restype = C.c_size_t
    lib.dnbx_layer_params_size.restype = C.c_size_t
    args = {
        "dnbx_init": [C.c_void_p, C.c_float],
        "dnbx_default_layer_params": [C.c_void_p],
        "dnbx_set_layer": [C.c_void_p, C.c_uint, C.c_void_p],
        "dnbx_note_on": [C.c_void_p, C.c_uint, C.c_uint],
        "dnbx_note_off": [C.c_void_p, C.c_uint],
        "dnbx_set_sustain": [C.c_void_p, C.c_uint],
        "dnbx_set_velocity_mode": [C.c_void_p, C.c_uint],
        "dnbx_render": [C.c_void_p, C.c_void_p, C.c_void_p, C.c_uint, C.c_void_p],
        "dnbx_all_off": [C.c_void_p],
    }
    for name, argtypes in args.items():
        getattr(lib, name).argtypes = argtypes
        getattr(lib, name).restype = C.c_int
    lib.dnbx_default_layer_params.restype = None
    require(lib.dnbx_part_size() == 2264, 'Required bounds/audio/state check failed')
    require(lib.dnbx_layer_params_size() == 96, 'Required bounds/audio/state check failed')
    return lib


def part(lib, rate=44100):
    result = (C.c_uint32 * (lib.dnbx_part_size() // 4))()
    require(lib.dnbx_init(result, rate) == 0, 'Required bounds/audio/state check failed')
    return result


def params(lib):
    result = (C.c_uint32 * 24)()
    lib.dnbx_default_layer_params(result)
    return result


def copy_params(p):
    return (C.c_uint32 * 24).from_buffer_copy(p)


def main():
    global OUT, TC
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True, help='New evidence directory')
    parser.add_argument('--toolchain', type=Path, help='Optional separately authorized target object check')
    args = parser.parse_args()
    OUT = args.output.resolve()
    TC = args.toolchain.resolve() if args.toolchain else None
    dirs = {"reference": REFERENCE, "dictionary": ROOT}
    expected = {'reference': {'dnbx_core.c': '3d8f898428b297478abb35692de56952451caed38cc02db53e16a19a54a7408b', 'dnbx_core.h': '86acaf17ee9c48dac151919098ab12de3acf457fab2bd9fdf57ac966f36c0962', 'fastmath.h': 'fbe052086010b17d8194b9cd3d61d3cb349cc1def3817ae9651a6683d7850b09'}, 'dictionary': {'dnbx_core.c': 'c370368d612410841ce812ec29c74038ff1f224de0047b282e6595a5e4a7ad84', 'dnbx_core.h': '86acaf17ee9c48dac151919098ab12de3acf457fab2bd9fdf57ac966f36c0962', 'fastmath.h': 'fbe052086010b17d8194b9cd3d61d3cb349cc1def3817ae9651a6683d7850b09'}}
    for label, directory in dirs.items():
        for name, expected_hash in expected[label].items():
            if digest(directory / name) != expected_hash:
                raise ValueError("Reviewed input changed: " + str(directory / name))
    OUT.mkdir(parents=True)  # Refuse to overwrite evidence.

    commands = []

    def run(argv, label):
        cp = subprocess.run(list(map(str, argv)), cwd=ROOT, capture_output=True, text=True)
        (OUT / (label + ".log")).write_text(cp.stdout + cp.stderr)
        commands.append({"label": label, "argv": list(map(str, argv)), "returncode": cp.returncode})
        cp.check_returncode()
        return cp.stdout

    libs = {}
    for name, src in dirs.items():
        so = OUT / (name + ".so")
        run(["cc", "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
             "-fno-fast-math", "-ffp-contract=off", "-DX0X_HOST", "-shared", "-fPIC",
             "-I" + str(src), src / "dnbx_core.c", "-o", so], name + "-host-build")
        libs[name] = load(so)

    boundary = []
    for layer in (0, 1):
        for field, (lo, hi) in enumerate(BOUNDS):
            lo32, hi32 = value(bits(lo)), value(bits(hi))
            for raw in samples(lo, hi):
                v = value(raw)
                expected = 0 if lo32 <= v <= hi32 else -2
                got, states, invalid_unchanged = [], [], True
                for lib in libs.values():
                    state = part(lib)
                    p = params(lib)
                    p[field] = raw  # Preserve SNaN payload and signed zeros exactly.
                    before = bytes(state)
                    got.append(lib.dnbx_set_layer(state, layer, p))
                    states.append(bytes(state))
                    if expected:
                        invalid_unchanged &= before == bytes(state)
                passed = got == [expected, expected] and states[0] == states[1] and invalid_unchanged
                boundary.append({"layer": layer, "field": field, "raw": f"{raw:08x}",
                                 "expected": expected, "got": got, "pass": passed})
                require(passed, boundary[-1])
    for mode in (0, 1, 2, 0xffffffff):
        got, states = [], []
        for lib in libs.values():
            s = part(lib)
            p = params(lib)
            p[23] = mode
            before = bytes(s)
            got.append(lib.dnbx_set_layer(s, 0, p))
            states.append(bytes(s))
            if mode > 1:
                require(before == bytes(s), 'Required bounds/audio/state check failed')
        require(got == ([0, 0] if mode <= 1 else [-2, -2]) and states[0] == states[1], 'Required bounds/audio/state check failed')

    # Bit-for-bit ongoing state/audio check with notes, sustain, waveform,
    # pitch and non-pitch edits, both independent parts, and irregular calls.
    renders, total_frames, hashes = 0, 0, {name: hashlib.sha256() for name in libs}
    for rate in (8000, 44100, 48000, 192000):
        states = {name: [part(lib, rate), part(lib, rate)] for name, lib in libs.items()}
        for name, lib in libs.items():
            for index, state in enumerate(states[name]):
                p = params(lib)
                for field, v in {0: .01013, 2: .05, 4: .0037, 5: .1, 6: .02017,
                                 8: .04, 12: .85, 19: 16, 20: .47, 22: index * .8 - .4}.items():
                    p[field] = bits(v)
                for layer in (0, 1):
                    require(lib.dnbx_set_layer(state, layer, p) == 0, 'Required bounds/audio/state check failed')
                for voice in range(6):
                    require(lib.dnbx_note_on(state, 60 + voice, 127 - voice * 12) == voice, 'Required bounds/audio/state check failed')
        for call, take in enumerate([1, 3, 5, 7, 13, 16, 64, 127, 256] * 8):
            for name, lib in libs.items():
                for state in states[name]:
                    if call == 3:
                        require(lib.dnbx_note_off(state, 60) == 0, 'Required bounds/audio/state check failed')
                    elif call == 7:
                        require(lib.dnbx_note_on(state, 99, 64) >= 0, 'Required bounds/audio/state check failed')
                    elif call == 11:
                        require(lib.dnbx_set_sustain(state, 1) == 0, 'Required bounds/audio/state check failed')
                    elif call == 19:
                        require(lib.dnbx_note_off(state, 61) == 0, 'Required bounds/audio/state check failed')
                    elif call == 23:
                        require(lib.dnbx_set_sustain(state, 0) == 0, 'Required bounds/audio/state check failed')
                    elif call == 31:
                        p = params(lib)
                        p[21], p[14], p[23] = bits(12), bits(1.32), 1
                        require(lib.dnbx_set_layer(state, 1, p) == 0, 'Required bounds/audio/state check failed')
                    elif call == 39:
                        require(lib.dnbx_set_velocity_mode(state, 1) == 0, 'Required bounds/audio/state check failed')
                    elif call == 47:
                        require(lib.dnbx_all_off(state) == 0, 'Required bounds/audio/state check failed')
                    elif call == 51:
                        require(lib.dnbx_note_on(state, 127, 127) >= 0, 'Required bounds/audio/state check failed')
            outputs = []
            for name, lib in libs.items():
                chunk = b""
                for state in states[name]:
                    left, right = (C.c_float * take)(), (C.c_float * take)()
                    require(lib.dnbx_render(state, left, right, take, None) == 0, 'Required bounds/audio/state check failed')
                    chunk += bytes(left) + bytes(right)
                hashes[name].update(chunk)
                outputs.append(chunk)
            require(outputs[0] == outputs[1], (rate, call, "audio"))
            for index in (0, 1):
                require(bytes(states["reference"][index]) == bytes(states["dictionary"][index]), (rate, call, index))
            renders += 2
            total_frames += take * 2

    # Negative control: incorrectly map pulse width to the unit interval.
    bad = OUT / "bad_dictionary.c"
    text = (dirs["dictionary"] / "dnbx_core.c").read_text()
    old = "0,0,0,1,0,0,0,0,0,1,0,0,1,2,3,4,5,1,6,0,7,8,9"
    require(text.count(old) == 1, 'Required bounds/audio/state check failed')
    bad.write_text(text.replace(old, old.replace("1,2,3,4", "1,1,3,4")))
    run(["cc", "-std=c11", "-O2", "-fno-fast-math", "-ffp-contract=off", "-DX0X_HOST",
         "-shared", "-fPIC", "-I" + str(dirs["dictionary"]), bad, "-o", OUT / "bad.so"], "negative-build")
    negative = load(OUT / "bad.so")
    nstate, np = part(negative), params(negative)
    np[13] = bits(0)
    rejected_negative = negative.dnbx_set_layer(nstate, 0, np) != -2
    require(rejected_negative, 'Required bounds/audio/state check failed')

    native = {}
    if TC is not None:
        for name, src in dirs.items():
            obj = OUT / (name + ".o")
            run([TC / "pi32v2/bin/clang", "-target", "pi32v2", "-mcpu=r3", "-mfprev1",
                 "-ffp-contract=off", "-ffunction-sections", "-fdata-sections", "-fno-builtin",
                 "-fno-fast-math", "-Wall", "-Wno-unused-function", "-std=c99", "-I" + str(src),
                 "-idirafter", TC / "pi32v2/include", "-Oz", "-DDNBX_DIAGNOSTIC_COUNTERS=1",
                 "-c", src / "dnbx_core.c", "-o", obj], name + "-target-compile")
            sections = run([TC / "common/bin/objdump", "-h", obj], name + "-target-sections")
            symbols = run([TC / "common/bin/objdump", "-t", obj], name + "-target-symbols")
            section_sizes = {}
            for match in re.finditer(r"^\s*\d+\s+(\.\S+)\s+([0-9a-f]+)\s+", sections, re.M):
                if match[1].startswith((".text", ".rodata")):
                    section_sizes[match[1]] = int(match[2], 16)
            native[name] = {"sha256": digest(obj), "text_plus_rodata_bytes": sum(section_sizes.values()),
                            "sections": section_sizes,
                            "validation_symbols": [line for line in symbols.splitlines()
                                                   if any(n in line for n in ("params_valid", "dnbx_set_layer"))]}
    result = {
        "created_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "source_sha256": {name: digest(src / "dnbx_core.c") for name, src in dirs.items()},
        "boundary_case_count": len(boundary), "boundary_cases": boundary,
        "oscillator_mode_cases": 4, "all_boundary_and_state_checks_pass": True,
        "total_rendered_frames_across_rates_and_parts": total_frames,
        "total_render_calls": renders, "rates": [8000, 44100, 48000, 192000],
        "audio_and_complete_state_bitwise_equal_after_all_calls": True,
        "audio_sha256": {k: v.hexdigest() for k, v in hashes.items()},
        "wrong_pulse_width_range_negative_control_rejected": rejected_negative,
        "target_objects": native,
        "target_object_delta_bytes": (native["dictionary"]["text_plus_rodata_bytes"] - native["reference"]["text_plus_rodata_bytes"]) if native else None,
        "native_object_execution": "EXECUTED" if native else "NOT_RUN",
        "native_full_image_fit": "NOT_TESTED; a stock-map full-closure link is required",
        "scope": "Only physical range representation changes. No interface/layout/synthesis arithmetic changes. Target object bytes are not firmware-image bytes; no callback timing measured.",
        "commands": commands,
    }
    (OUT / "RESULT.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({k: v for k, v in result.items() if k not in ("boundary_cases", "commands", "target_objects")}, indent=2))


if __name__ == "__main__":
    main()
