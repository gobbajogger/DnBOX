#!/usr/bin/env python3
"""Measure the shipping 909 directly in its linked PI32v2 firmware."""
import argparse
import os
from pathlib import Path
import subprocess
import struct

HERE = Path(__file__).resolve().parent
SYMBOLS = ["d909", "drum909_init", "drum909_trigger", "drum909_render_st_cleared"]

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
    for label in SYMBOLS:
        script += f"\n{label} = 0x{symbols[label]:x};"
    (out / "linked.ld").write_text(script + "\n")

def main():
    args = argparse.ArgumentParser(description=__doc__)
    for name in ("repo", "elf", "compiler", "gen", "out"):
        args.add_argument("--"+name.replace("_","-"), dest=name, type=lambda p:Path(p).resolve(), required=True)
    for name in ("cpu", "rustc", "core_rlib"):
        args.add_argument("--" + name.replace("_", "-"), dest=name, type=lambda p: Path(p).resolve())
    a = args.parse_args()
    if not a.cpu and not (a.rustc and a.core_rlib):
        args.error("provide --cpu or both --rustc and --core-rlib")
    a.out.mkdir(parents=True, exist_ok=True)
    if not Path(os.environ.get("TMPDIR","/tmp")).is_dir():
        os.environ["TMPDIR"] = str(a.out)
    prepare(a.elf, a.out)
    def run(*cmd):
        subprocess.run(cmd, cwd=a.out, check=True)
    cc = a.compiler / "bin/clang"
    flags = ["-target","pi32v2","-mcpu=r3","-mfprev1","-ffp-contract=off","-fno-builtin",
             "-ffunction-sections","-fdata-sections","-Os","-I",a.gen]
    for include in ("firmware/src","firmware/src/app","firmware/src/dsp",
                    "experiments/original_synth","experiments/wobble"):
        flags.extend(["-I", a.repo / include])
    flags.extend(["-idirafter", a.compiler / "include"])
    run(cc, *flags, "-c", HERE/"entry.S", "-o", "entry.o")
    cpu = a.cpu or a.out / "cpu"
    if not a.cpu:
        run(a.rustc,"-O","--edition=2021","--extern",f"fm1_core={a.core_rlib}","-L",
            f"dependency={a.core_rlib.parent}",HERE/"cpu.rs","-o",cpu)
    for mask in (0,28,1920,2047):
        run(cc,*flags,f"-DMASK={mask}","-c",HERE/"drive.c","-o","drive.o")
        run(a.compiler/"bin/ld","--gc-sections","-e","_start","-T","linked.ld","-o","drive.elf","entry.o","drive.o")
        run(a.compiler/"bin/objcopy","-j",".text","-O","binary","drive.elf","drive.bin")
        syms=subprocess.check_output(["nm",a.out/"drive.elf"],text=True)
        measured=next(line.split()[0] for line in syms.splitlines() if line.endswith(" measured"))
        run(cpu,"segments.bin","drive.bin",measured)

if __name__ == "__main__":
    main()
