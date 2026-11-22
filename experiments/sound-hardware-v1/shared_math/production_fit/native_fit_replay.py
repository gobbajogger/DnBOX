#!/usr/bin/env python3
"""Prepare/run an isolated full-closure native fit experiment on the stock map.

Preparation copies source and captures the official compilation plan, but starts
no compiler, linker, emulator, or timer. Execute only after the Sound owner has
explicitly released the shared heavy slot. There is no enlarged-map fallback.
"""
from pathlib import Path
import argparse
import datetime
import hashlib
import importlib.util
import json
import os
import re
import shutil
import subprocess
import sys

HERE = Path(__file__).resolve().parent
RECOVERED = Path("/workspace/scratch/67e685478953/superj6_research")
EXPERIMENT = RECOVERED / "resource_review/dnbx_core_7161c21e6d9d773c"
TOOLCHAIN = RECOVERED / "build_inputs/extracted/jieli-linux-toolchains-20260730.1"
FAST_MATH_SHA = "fbe052086010b17d8194b9cd3d61d3cb349cc1def3817ae9651a6683d7850b09"
XIP_START = 0x02000120
XIP_CAPACITY = 552672
XIP_RESERVE = 1024


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def json_write(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")


def local(path):
    path = path.resolve()
    if not path.is_relative_to(HERE):
        raise ValueError("Every output must remain within fit-work: " + str(path))
    return path


def source_manifest(tree):
    # Generated headers are real inputs and are included. Existing objects and
    # prior native images are excluded; execution recompiles every app unit.
    suffixes = {".c", ".h", ".S", ".s", ".inc", ".ld", ".py"}
    return {str(p.relative_to(tree)): digest(p)
            for p in sorted(tree.rglob("*"))
            if p.is_file() and p.suffix in suffixes and "__pycache__" not in p.parts}


def prepare(args):
    if not re.fullmatch(r"[a-zA-Z0-9][a-zA-Z0-9._-]*", args.label):
        raise ValueError("Use a simple unique label")
    run = local(HERE / "native-plans" / args.label)
    run.mkdir(parents=True)  # Refuse to overwrite a previous experiment.
    source = args.input_tree or (EXPERIMENT / "replacement-tree" if args.composition == "compact"
                                 else RECOVERED / "resource_review/native-baseline-76040")
    source = source.resolve()
    tree = run / "tree"
    shutil.copytree(source, tree)
    dsp = tree / "firmware/src/dsp"
    if digest(dsp / "fastmath.h") != FAST_MATH_SHA:
        raise ValueError("Input fastmath.h is not the reviewed source; review the patch against its actual bytes")
    modified = []
    if args.composition == "compact":
        for name in ("dnbx_core.c", "dnbx_core.h", "juno6.c", "juno6.h"):
            p = dsp / name
            new = (HERE / "reference" / name).read_bytes()
            if not p.exists() or p.read_bytes() != new:
                modified.append(str(p.relative_to(tree)))
            p.write_bytes(new)
    if args.profile == "shared":
        shutil.copy2(HERE / "candidate/fastmath.h", dsp / "fastmath.h")
        modified.append("firmware/src/dsp/fastmath.h")

    # Read only local copies of build modules. Capture at the first compile
    # batch in build_app, before a compiler or linker can run.
    sys.dont_write_bytecode = True
    sys.path.insert(0, str(tree / "tools"))
    spec = importlib.util.spec_from_file_location("isolated_native_build", tree / "tools/build.py")
    build = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(build)
    build.BOARD = "smk37"
    build.MODEL = args.model
    build.OUT = tree / "build" / ("smk37-elite" if args.model == "elite" else "smk37")
    build.PRODUCT = "SMK-37 " + ("Elite" if args.model == "elite" else "Pro") + "_900"
    build.BOARD_DEFS["smk37"] = ["-DBOARD_SMK37=1"]
    if args.model == "elite":
        build.BOARD_DEFS["smk37"].append('-DSMK_MODEL="SMK-37 Elite"')
    os.environ["JIELI_TOOLCHAIN"] = str(args.toolchain.resolve())
    captured = []

    class PlanCaptured(Exception):
        pass

    def capture(*units):
        captured.extend(units)
        raise PlanCaptured()

    def forbidden(*unused, **kw):
        raise AssertionError("Preparation attempted to execute a native tool")

    build.tc_all = capture
    build.tc = forbidden
    old_cwd = Path.cwd()
    os.chdir(tree)
    try:
        try:
            build.build_app()
        except PlanCaptured:
            pass
    finally:
        os.chdir(old_cwd)
    assert captured, "No compilation commands captured"
    commands = []
    objects = []
    for unit in captured:
        assert unit[0] == "cc"
        argv = [str(a.relative_to(tree)) if isinstance(a, Path) and a.is_absolute() else str(a)
                for a in unit[1:]]
        src = Path(argv[argv.index("-c") + 1])
        obj = Path(argv[argv.index("-o") + 1])
        assert not obj.is_absolute() and (tree / obj).resolve().is_relative_to(tree)
        if args.composition == "compact" and src.stem in {"juno6", "dnbx_core"}:
            # Use the exact retained compact experiment's dedicated flags,
            # rather than assuming its -Oz comparison used the app defaults.
            argv = ["-mcpu=r3", "-mfprev1", "-ffp-contract=off", "-ffunction-sections",
                    "-fdata-sections", "-fno-builtin", "-fno-fast-math", "-Wall",
                    "-Wno-unused-function", "-std=c99", "-Ifirmware/src/dsp",
                    "-idirafter", str(args.toolchain / "pi32v2/include"), "-Oz",
                    "-DDNBX_DIAGNOSTIC_COUNTERS=1", "-flto", "-c", str(src), "-o", str(obj)]
        commands.append({"label": "compile-" + str(obj).replace("/", "_"),
                         "argv": [str(args.toolchain / "pi32v2/bin/clang"), "-target", "pi32v2", *argv]})
        objects.append(str(obj))
    if args.composition == "compact":
        assert any(Path(x).stem == "dnbx_core" for x in objects), "Compact fixture build plan lacks dnbx_core"

    script = build.OUT / "app.ld"
    text = script.read_text()
    match = re.search(r"XIP\s*\(rx\)\s*:\s*ORIGIN\s*=\s*(0x[0-9a-fA-F]+),\s*LENGTH\s*=\s*(0x[0-9a-fA-F]+)", text)
    assert match and int(match[1], 16) == XIP_START and int(match[2], 16) == XIP_CAPACITY
    elf = run / "app.elf"
    map_file = run / "app.map"
    link = [str(args.toolchain / "pi32v2/bin/ld"),
            "-plugin", str(args.toolchain / "pi32v2/bin/LLVMgold.so"),
            "-plugin-opt=-inline-threshold=0", "-plugin-opt=-inlinehint-threshold=0",
            "-plugin-opt=-inlinecold-threshold=0", "-plugin-opt=-enable-tail-merge",
            "-plugin-opt=-tail-merge-size=1", "-plugin-opt=-tail-merge-threshold=10000",
            "-plugin-opt=-function-sections", "-plugin-opt=-data-sections",
            "-plugin-opt=-fp-contract=off", "--sort-section=alignment", "--gc-sections", "-e", "_start",
            "-T", str(script), "-Map", str(map_file)]
    if args.composition == "compact":
        link += ["-u", "dnbx_diagnostic_counters_enabled"]
    link += [*objects, "-o", str(elf)]
    plan = {
        "schema": 1, "created_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "status": "PREPARED_NOT_RUN", "input_tree": str(source), "tree": str(tree),
        "composition": args.composition, "profile": args.profile, "model": args.model,
        "toolchain": str(args.toolchain), "compiler_sha256": digest(args.toolchain / "pi32v2/bin/clang"),
        "linker_sha256": digest(args.toolchain / "pi32v2/bin/ld"),
        "modified_source_paths": modified, "compile_commands": commands,
        "link_command": link, "link_script": str(script), "link_script_sha256": digest(script),
        "source_manifest": source_manifest(tree), "elf": str(elf), "map": str(map_file),
        "physical_xip_capacity": XIP_CAPACITY, "required_free_bytes": XIP_RESERVE,
        "native_execution": "NOT_RUN", "emulator_execution": "NOT_RUN",
        "scope": "Full official app compilation closure, compact core/facade at their retained -Oz profile. Link only the unchanged physical stock map. No package, device write, diagnostic map enlargement, or acceptance inferred from a component object.",
    }
    json_write(run / "PLAN.json", plan)
    print(json.dumps({"status": plan["status"], "plan": str(run / "PLAN.json"),
                      "compile_units": len(commands), "modified_source_paths": modified}, indent=2))


def map_sections(path):
    result = {}
    if path.is_file():
        for m in re.finditer(r"^(\.\S+)\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)(?:\s+load address 0x([0-9a-fA-F]+))?", path.read_text(), re.M):
            result[m[1]] = {"vma": int(m[2], 16), "bytes": int(m[3], 16),
                            "lma": int(m[4] or m[2], 16)}
    return result


def execute(args):
    plan_path = local(args.plan)
    plan = json.loads(plan_path.read_text())
    run = plan_path.parent
    result_file = run / "RESULT.json"
    if result_file.exists() or (run / "COMMANDS.json").exists():
        raise ValueError("Refuse to overwrite an executed experiment; prepare a fresh label")
    tree = local(Path(plan["tree"]))
    assert source_manifest(tree) == plan["source_manifest"], "Prepared sources changed"
    assert digest(Path(plan["link_script"])) == plan["link_script_sha256"]
    tc = Path(plan["toolchain"])
    assert digest(tc / "pi32v2/bin/clang") == plan["compiler_sha256"]
    assert digest(tc / "pi32v2/bin/ld") == plan["linker_sha256"]
    receipt = []

    def invoke(command):
        start = datetime.datetime.now(datetime.timezone.utc).isoformat()
        cp = subprocess.run(command["argv"], cwd=tree, text=True, capture_output=True)
        (run / (command["label"] + ".log")).write_text(cp.stdout + cp.stderr)
        receipt.append({**command, "start_utc": start,
                        "end_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                        "returncode": cp.returncode})
        json_write(run / "COMMANDS.json", receipt)
        print(command["label"] + " exit=" + str(cp.returncode), flush=True)
        return cp

    result = {"status": "RUNNING", "plan_sha256": digest(plan_path),
              "composition": plan["composition"], "profile": plan["profile"], "model": plan["model"],
              "physical_xip_capacity": XIP_CAPACITY, "required_free_bytes": XIP_RESERVE,
              "native_execution": "NOT_RUN", "emulator_execution": "NOT_RUN"}
    json_write(result_file, result)
    for command in plan["compile_commands"]:
        cp = invoke(command)
        if cp.returncode:
            result.update(status="COMPILE_FAILED", failed_command=command["label"])
            json_write(result_file, result)
            return 2
    cp = invoke({"label": "physical-link", "argv": plan["link_command"]})
    result["physical_link_returncode"] = cp.returncode
    sections = map_sections(Path(plan["map"]))
    result["sections_from_physical_map"] = sections
    if ".pool" in sections:
        free = 344064 - sections[".pool"]["bytes"]
        result["pool_raw_free_bytes"] = free
        result["pool_margin_above_8192_reserve"] = free - 8192
        result["pool_gate"] = "PASS" if free - 8192 >= 512 else "FAIL"
    if ".data" in sections and ".bss" in sections:
        end = max(sections[n]["vma"] + sections[n]["bytes"] for n in (".data", ".bss"))
        result["ordinary_ram_extent_bytes"] = end - 0x01C08000
        result["ordinary_ram_free_bytes"] = 98304 - result["ordinary_ram_extent_bytes"]
    if ".xpool" in sections:
        result["xpool_free_bytes"] = 9216 - sections[".xpool"]["bytes"]
    loads = (".text", ".ram_text", ".c1_text", ".data")
    if all(name in sections for name in loads):
        end = max(sections[name]["lma"] + sections[name]["bytes"] for name in loads)
        image_bytes = ((end - XIP_START + 3) // 4) * 4 + 4
        result.update(load_image_bytes=image_bytes, physical_free_bytes=XIP_CAPACITY - image_bytes,
                      reserve_margin_bytes=XIP_CAPACITY - XIP_RESERVE - image_bytes,
                      image_size_source="physical link map, aligned load sections plus four-byte marker")
    else:
        result["image_size_source"] = "UNAVAILABLE: physical linker map lacked a load section; no enlarged fallback"
    if cp.returncode == 0:
        elf = Path(plan["elf"])
        result["elf_sha256"] = digest(elf)
        for flag, label in (("-t", "symbols"), ("-h", "sections"), ("-d", "disassembly")):
            p = invoke({"label": label, "argv": [str(tc / "common/bin/objdump"), flag, str(elf)]})
            if p.returncode:
                result.update(status="INSPECTION_FAILED", failed_command=label)
                json_write(result_file, result)
                return 2
            if flag == "-t":
                result["undefined"] = [line.strip() for line in p.stdout.splitlines()
                                       if "*UND*" in line and line.strip().split()[-1] != "00000000"]
                result["soft_double_symbols"] = sorted(set(re.findall(
                    r"\s(__(?:add|sub|mul|div|neg|cmp|eq|ne|lt|le|gt|ge|un|extendsf|truncdf|fixdf|fixunsdf|floatsidf|floatunsidf|floatdidf)\w*df\w*)$", p.stdout, re.M)))
    result["xip_gate"] = "PASS" if cp.returncode == 0 and result.get("reserve_margin_bytes", -1) >= 0 else "FAIL"
    result["status"] = "LINK_MEASURED" if cp.returncode == 0 else "PHYSICAL_LINK_FAILED"
    result["qualification"] = "Resource experiment only. Header sharing and actual target behavior/timing require independent target-code evidence."
    json_write(result_file, result)
    print(json.dumps(result, indent=2))
    return 0 if result["xip_gate"] == "PASS" else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    p = commands.add_parser("prepare", help="copy inputs and capture commands without any native execution")
    p.add_argument("--label", required=True)
    p.add_argument("--composition", choices=("stock", "compact"), default="compact")
    p.add_argument("--profile", choices=("reference", "shared"), required=True)
    p.add_argument("--model", choices=("elite", "pro"), default="elite")
    p.add_argument("--input-tree", type=Path)
    p.add_argument("--toolchain", type=Path, default=TOOLCHAIN)
    q = commands.add_parser("execute", help="start native compilation/link only after explicit owner heavy-slot release")
    q.add_argument("--plan", type=Path, required=True)
    args = parser.parse_args()
    if args.command == "prepare":
        prepare(args)
        return 0
    return execute(args)


if __name__ == "__main__":
    raise SystemExit(main())
