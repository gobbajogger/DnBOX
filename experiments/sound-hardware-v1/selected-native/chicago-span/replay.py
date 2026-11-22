#!/usr/bin/env python3
"""Replay the Chicago job directly in an immutable linked PI32v2 firmware."""
import argparse
import os
from pathlib import Path
import subprocess
import struct

HERE = Path(__file__).resolve().parent
SYMBOLS = """proj engine_init engine_sound_defaults original_bass original_runtime_init
original_runtime_prepare original_runtime_retire original_runtime_key original_sound_init
original_sound_set workspace_b original_switch_gain bass_accent_mk bass_accent_mk_to
eng_c1_mask eng_c1_next eng_job_arg core2_jobs original_fx_owner original_tuning original_fx""".split()


def prepare(elf, out):
    image = elf.read_bytes()
    assert image[:6] == b"\x7fELF\x01\x01"
    table = struct.unpack_from("<I", image, 32)[0]
    stride, count, string_table = struct.unpack_from("<3H", image, 46)
    sections = [struct.unpack_from("<10I", image, table + i * stride) for i in range(count)]

    def section_bytes(index):
        section = sections[index]
        return image[section[4]:section[4] + section[5]]

    def name(strings, start):
        return strings[start:strings.index(b"\0", start)].decode()

    sections_by_name = section_bytes(string_table)
    symbols = {}
    with (out / "segments.bin").open("wb") as segments:
        for i, section in enumerate(sections):
            if section[1] == 1 and section[2] & 2 and section[3]:
                segments.write(struct.pack("<II", section[3], section[5]))
                segments.write(section_bytes(i))
            if name(sections_by_name, section[0]) == ".text":
                assert section[3] + section[5] <= 0x02090000
            if section[1] == 2:
                names = section_bytes(section[6])
                for offset in range(section[4], section[4]+section[5], section[9]):
                    label, value = struct.unpack_from("<II", image, offset)
                    if value:
                        symbols[name(names,label)] = value
    assert symbols["_c1_ustack"] == 0x01c74000
    assert symbols["_c1_ustack_top"] == 0x01c75100
    assert symbols["_c1_sstack_top"] == 0x01c75200
    script = (HERE / "memory.ld").read_text()
    stage = "eng_stage" in symbols
    for label in SYMBOLS + (["eng_stage","eng_808_split"] if stage else []):
        script += f"\n{label} = 0x{symbols[label]:x};"
    (out / "linked.ld").write_text(script + "\n")
    return stage


def main():
    args = argparse.ArgumentParser(description=__doc__)
    for name in ("repo", "elf", "compiler", "cpu", "gen", "out"):
        args.add_argument("--"+name.replace("_","-"), dest=name, type=lambda p:Path(p).resolve(), required=True)
    args.add_argument("--one", nargs=3, metavar=("FACTORY","PHASE","FX"))
    a = args.parse_args()
    a.out.mkdir(parents=True, exist_ok=True)
    if not Path(os.environ.get("TMPDIR","/tmp")).is_dir():
        os.environ["TMPDIR"] = str(a.out)
    legacy = prepare(a.elf, a.out)
    def run(*cmd):
        subprocess.run(cmd, cwd=a.out, check=True)
    cc = a.compiler / "bin/clang"
    flags = ["-target","pi32v2","-mcpu=r3","-mfprev1","-ffp-contract=off","-fno-builtin",
             "-ffunction-sections","-fdata-sections","-Os","-I",a.gen]
    for include in ("firmware/src","firmware/src/app","firmware/src/dsp",
                    "experiments/original_synth","experiments/wobble"):
        flags.extend(["-I", a.repo / include])
    flags.extend(["-idirafter", a.compiler / "include"])
    run(cc,*flags,f"-DLEGACY_STAGE={int(legacy)}","-c",HERE/"drive.c","-o","drive.o")
    run(cc,*flags,"-c",HERE/"entry.S","-o","entry.o")
    run(a.compiler/"bin/ld","--gc-sections","-e","_start","-T","linked.ld","-o","drive.elf","entry.o","drive.o")
    run(a.compiler/"bin/objcopy","-j",".text","-O","binary","drive.elf","drive.bin")
    syms=subprocess.check_output(["nm",a.out/"drive.elf"],text=True)
    measured=next(line.split()[0] for line in syms.splitlines() if line.endswith(" measured"))
    with (a.out/"results.csv").open("w") as output:
        subprocess.run([a.cpu,"segments.bin","drive.bin",measured,*(a.one or [])],
                       cwd=a.out,stdout=output,check=True)

if __name__ == "__main__":
    main()
