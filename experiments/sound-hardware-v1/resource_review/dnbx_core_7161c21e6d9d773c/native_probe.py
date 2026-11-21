#!/usr/bin/env python3
"""Native resource-only composition; never runs or installs target binaries.

Existing stock objects are retained unchanged. Every public dnbx API is a
link root so dead stripping cannot turn this into an unrealistically small
result. Failed physical-map links remain FAIL. An enlarged-map diagnostic
may be produced only to count overflowing bytes; it is never a stock image.
"""
from pathlib import Path
import hashlib
import importlib.util
import json
import os
import re
import struct
import subprocess
import sys

HERE = Path(__file__).resolve().parent
REVIEW = HERE.parent
ROOT = REVIEW.parent
BASE = REVIEW / "native-baseline-76040"
TC = ROOT / "build_inputs/extracted/jieli-linux-toolchains-20260730.1"
OUT = HERE / "native"
OUT.mkdir(exist_ok=True)
COMMANDS = []

def run(args, label, check=False):
    args = list(map(str, args))
    p = subprocess.run(args, cwd=BASE, text=True, capture_output=True)
    COMMANDS.append({"label": label, "cwd": str(BASE), "argv": args,
                     "returncode": p.returncode})
    (OUT / (label + ".log")).write_text(p.stdout + p.stderr)
    if check and p.returncode:
        raise RuntimeError(label + " failed: " + p.stderr[-2000:])
    return p

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def parse_sections(text):
    return {m.group(1): {"bytes": int(m.group(2), 16),
                         "address": int(m.group(3), 16)}
            for m in re.finditer(r"^\s*\d+\s+(\.\S+)\s+([0-9a-f]+)\s+([0-9a-f]+)", text, re.M)}

def parse_symbols(text):
    return [{"address": int(m.group(1), 16), "section": m.group(2),
             "bytes": int(m.group(3), 16), "name": m.group(4)}
            for m in re.finditer(r"^([0-9a-f]+)\s+.*?(\.[A-Za-z_][\w.]*)\s+([0-9a-f]+)\s+(\S+)\s*$", text, re.M)]

def inspect(path, label):
    h = run([TC / "common/bin/objdump", "-h", path], label + "-sections", True).stdout
    t = run([TC / "common/bin/objdump", "-t", path], label + "-symbols", True).stdout
    run([TC / "common/bin/objdump", "-d", path], label + "-disassembly", True)
    undef = [line.strip() for line in t.splitlines() if "*UND*" in line and line.strip().split()[-1] != "00000000"]
    return {"sha256": sha(path), "sections": parse_sections(h),
            "symbols": parse_symbols(t), "undefined": undef}

provenance = json.loads((HERE / "CANDIDATE-PROVENANCE.json").read_text())
for row in provenance["files"]:
    if sha(Path(row["snapshot_path"])) != row["sha256"]:
        raise RuntimeError("snapshot changed")
provenance["fastmath"] = {"path": str(BASE / "firmware/src/dsp/fastmath.h"),
                           "sha256": sha(BASE / "firmware/src/dsp/fastmath.h")}

flags = ["-target", "pi32v2", "-mcpu=r3", "-mfprev1", "-ffp-contract=off",
         "-ffunction-sections", "-fdata-sections", "-fno-builtin", "-fno-fast-math",
         "-Wall", "-Wno-unused-function", "-std=c99", "-I" + str(HERE / "src"),
         "-I" + str(BASE / "firmware/src/dsp"), "-idirafter", str(TC / "pi32v2/include")]
compiler = TC / "pi32v2/bin/clang"
objects = {}
for opt in ["Os", "O2"]:
    obj = OUT / ("core-" + opt + ".o")
    run([compiler, *flags, "-" + opt, "-c", HERE / "src/dnbx_core.c", "-o", obj], "compile-" + opt, True)
    objects[opt] = inspect(obj, "core-" + opt)
    run([compiler, *flags, "-" + opt, "-S", HERE / "src/dnbx_core.c", "-o", OUT / ("core-" + opt + ".s")], "assembly-" + opt, True)
    run([compiler, *flags, "-" + opt, "-flto", "-c", HERE / "src/dnbx_core.c", "-o", OUT / ("core-" + opt + "-lto.o")], "compile-" + opt + "-lto", True)

names = ["pointer", "float", "double", "envelope_params", "layer_params", "envelope_state",
         "layer_voice", "voice", "layer", "part", "work", "voice_snapshot", "part_layer_offset",
         "part_voice_offset", "part_raw_offset", "part_raw_present_offset", "backend_stock",
         "runtime_part_stock", "selected_fx_stock", "chorus_stock", "candidate_plus_chorus",
         "slot_backend_with_candidate", "two_slot_backend_bytes"]
expressions = ["sizeof(void*)", "sizeof(float)", "sizeof(double)",
    "sizeof(dnbx_envelope_params)", "sizeof(dnbx_layer_params)", "sizeof(dnbx_envelope_state)",
    "sizeof(dnbx_layer_voice)", "sizeof(dnbx_voice)", "sizeof(dnbx_layer)", "sizeof(dnbx_part)",
    "sizeof(dnbx_work)", "sizeof(dnbx_voice_snapshot)", "offsetof(dnbx_part,layer)",
    "offsetof(dnbx_part,voice)", "offsetof(dnbx_part,raw)", "offsetof(dnbx_part,raw_present)",
    "sizeof(original_runtime_backend)", "sizeof(original_runtime_part)",
    "sizeof(original_chicago_fx2_compact)", "sizeof(juno6_chorus_t)",
    "sizeof(struct candidate_with_chorus)", "sizeof(union candidate_backend)",
    "2*sizeof(union candidate_backend)"]
layout_source = '''#include <stddef.h>
#include <stdint.h>
#include "dnbx_core.h"
#include "firmware/src/app/original_runtime.h"
#include "experiments/original_synth/chicago_fx2_compact.h"
struct candidate_with_chorus { dnbx_part core; juno6_chorus_t chorus; };
union candidate_backend { original_runtime_backend stock; struct candidate_with_chorus candidate; };
const uint32_t candidate_layout[] = {''' + ",".join(expressions) + "};\n"
(OUT / "layout.c").write_text(layout_source)
run([compiler, *flags, "-Os", "-I" + str(BASE), "-I" + str(BASE / "experiments/original_synth"),
     "-I" + str(BASE / "experiments/wobble"), "-I" + str(BASE / "build/gen"),
     "-c", OUT / "layout.c", "-o", OUT / "layout.o"], "layout-compile", True)
run([TC / "common/bin/objcopy", "-O", "binary", "-j", ".rodata.candidate_layout",
     OUT / "layout.o", OUT / "layout.bin"], "layout-extract", True)
raw = (OUT / "layout.bin").read_bytes()
layout = dict(zip(names, struct.unpack("<" + "I" * (len(raw) // 4), raw)))
if len(layout) != len(names):
    raise RuntimeError("layout section mismatch")

sys.path.insert(0, str(BASE / "tools"))
spec = importlib.util.spec_from_file_location("native_baseline_build", BASE / "tools/build.py")
build = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build)
os.environ["JIELI_TOOLCHAIN"] = str(TC)
build.BOARD = "smk37"
roots = re.findall(r"^(?:uint32_t|size_t|void|int)\s+(dnbx_\w+)\s*\(", (HERE / "src/dnbx_core.h").read_text(), re.M)
link_flags = ["-plugin", str(TC / "pi32v2/bin/LLVMgold.so"),
              "-plugin-opt=-inline-threshold=0", "-plugin-opt=-inlinehint-threshold=0",
              "-plugin-opt=-inlinecold-threshold=0", "-plugin-opt=-enable-tail-merge",
              "-plugin-opt=-tail-merge-size=1", "-plugin-opt=-tail-merge-threshold=10000",
              "-plugin-opt=-function-sections", "-plugin-opt=-data-sections",
              "-plugin-opt=-fp-contract=off", "--sort-section=alignment",
              "--gc-sections", "-e", "_start"]
links = []
for opt in ["Os", "O2"]:
  for model, board_dir in [("elite", "smk37-elite"), ("pro", "smk37")]:
    build.MODEL = model
    build.OUT = BASE / "build" / board_dir
    _, objs = build.app_compile_commands([], True, "0")
    if any(not x.is_file() for x in objs):
        raise RuntimeError("baseline object missing")
    script = build.OUT / "app.ld"
    label = "additive-" + model + "-" + opt
    target = OUT / (label + ".elf")
    cmd = [TC / "pi32v2/bin/ld", *link_flags, "-T", script,
           *[a for root in roots for a in ["-u", root]], *objs,
           OUT / ("core-" + opt + "-lto.o"), "-o", target]
    p = run(cmd, label + "-physical-link")
    row = {"model": model, "optimization": opt, "physical_link_returncode": p.returncode,
           "scope": "Stock image plus all public candidate APIs; stock Juno retained; no candidate runtime instance or routing installed",
           "public_link_roots": roots}
    if p.returncode:
        larger = OUT / (label + "-DIAGNOSTIC-OVERFLOW.ld")
        txt = script.read_text()
        expected = "LENGTH = 0x86EE0"
        if expected not in txt:
            raise RuntimeError("unexpected physical XIP script")
        larger.write_text(txt.replace(expected, "LENGTH = 0xA0000"))
        diagnostic = OUT / (label + "-DIAGNOSTIC-OVERFLOW.elf")
        dc = list(cmd)
        dc[dc.index("-T") + 1] = larger
        dc[dc.index("-o") + 1] = diagnostic
        run(dc, label + "-diagnostic-overflow-link", True)
        target = diagnostic
        row["diagnostic_only"] = True
    else:
        row["diagnostic_only"] = False
    details = inspect(target, label)
    sections = details["sections"]
    image = sum(sections[k]["bytes"] for k in [".text", ".ram_text", ".c1_text", ".data"]) + 4
    # Same linked load section ordering/alignment as the unchanged build script.
    row.update({"elf": str(target), "elf_sha256": details["sha256"],
                "load_image_bytes_including_end_marker": image,
                "physical_xip_capacity": 552672,
                "physical_xip_free": 552672 - image,
                "required_free": 1024,
                "xip_gate": "PASS" if 552672 - image >= 1024 else "FAIL",
                "sections": sections, "symbols": details["symbols"],
                "undefined": details["undefined"]})
    links.append(row)
    print(json.dumps({k: v for k, v in row.items() if k not in ["symbols", "sections", "public_link_roots"]}), flush=True)

result = {"provenance": provenance, "flags": flags, "native_layout": layout,
          "objects_without_lto": objects, "full_additive_links": links,
          "not_measured": ["native execution time", "whole callback stack high water", "full engine behavior", "original sound fidelity", "full control binding", "complete FX graph"]}
(OUT / "RESULT.json").write_text(json.dumps(result, indent=2))
(OUT / "COMMANDS.json").write_text(json.dumps(COMMANDS, indent=2))
print(json.dumps({"layout": layout, "commands": len(COMMANDS)}, indent=2))
