#!/usr/bin/env python3
"""Capture isolated production app plans. Never launches a native tool.

This is a preparation-only adapter for the reviewed native_fit_replay.py.
All outputs stay below this script's directory; the canonical tree is read-only.
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
HEAD = "e0ef1994a91abef619c755f9428e44c8c9f064c4"
BASELINE = Path("/workspace/scratch/67e685478953/superj6_research/resource_review/native-baseline-76040")
TOOLCHAIN = Path("/workspace/scratch/67e685478953/superj6_research/build_inputs/extracted/jieli-linux-toolchains-20260730.1")
REPLAY = Path("/workspace/scratch/de30bbe568fe/fit-work/native_fit_replay.py")
GENERATED = {
    "chicago_compact_private.h": "22beb20b008a93a22000752e62b25018b793bae5f69b1aca5d06c8152916e189",
    "felucca_font.h": "5d895596344a94798b95b86137805bdb685b2e0263f20d2824dea37dbf99e352",
    "x0x_break_bank.h": "b114be3c1c1f2019f6b920a732a17d4ce47ba203ac3505a93c65c67da1e2a3f0",
    "x0x_drum_samples.h": "50e7aa406f7767d7b10bb2165aa9d19e59b27b451dea59b881cab06a0c5818e9",
    "x0x_drum_tables.h": "9235c69643359a1d9e4f692e754d25e8b9a4014b9e41886eed995d8aaf100eb5",
}
FUNCTIONS = ("fm_exp2_nf", "fm_exp2f", "fm_expf", "fm_log2f", "fm_logf", "fm_db2lin")
SUFFIXES = {".c", ".h", ".S", ".s", ".inc", ".ld", ".py"}

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def write(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")

def files(root):
    return [p for p in sorted(root.rglob("*")) if p.is_file() and
            ".git" not in p.parts and "__pycache__" not in p.parts]

def manifest(root, suffixes=False):
    return {str(p.relative_to(root)): sha(p) for p in files(root)
            if not suffixes or p.suffix in SUFFIXES}

def bare_tokens(text):
    text = re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)
    return re.sub(r"\s+", "", text)

def forbidden(*args, **kwargs):
    raise RuntimeError("Preparation may not launch subprocesses or native tools")

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--source", type=Path, required=True)
    ap.add_argument("--baseline", type=Path, default=BASELINE)
    ap.add_argument("--toolchain", type=Path, default=TOOLCHAIN)
    args = ap.parse_args()
    source, baseline, tc = args.source.resolve(), args.baseline.resolve(), args.toolchain.resolve()
    head = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=source, text=True).strip()
    tree_id = subprocess.check_output(["git", "rev-parse", "HEAD^{tree}"], cwd=source, text=True).strip()
    assert head == HEAD, "Source commit changed; re-review exact input scope"
    assert not subprocess.check_output(["git", "status", "--porcelain"], cwd=source), "Canonical source is not clean"
    old_files = [p for p in files(baseline) if p.suffix in SUFFIXES]
    selected = [str(p.relative_to(baseline)) for p in old_files if not str(p.relative_to(baseline)).startswith("build/")]
    missing = [p for p in selected if not (source / p).is_file()]
    assert not missing, missing
    changed = [p for p in selected if sha(source / p) != sha(baseline / p)]
    assert changed == ["firmware/src/dsp/fastmath.h"], changed
    selected.append("experiments/original_synth/EXTERNAL-INPUTS.json")
    current_header = (source / "firmware/src/dsp/fastmath.h").read_text()
    reference_header = current_header
    for function in FUNCTIONS:
        reference_header, n = re.subn(r"\bFM_ONE float " + function + r"\(", "static inline float " + function + "(", reference_header)
        assert n == 1, (function, n)
    assert bare_tokens(reference_header) == bare_tokens((baseline / "firmware/src/dsp/fastmath.h").read_text())
    for name, digest in GENERATED.items():
        assert sha(baseline / "build/gen" / name) == digest, name
    tool_pins = {p: sha(tc / p) for p in ("pi32v2/bin/clang", "pi32v2/bin/ld", "pi32v2/bin/LLVMgold.so", "common/bin/objdump")}
    tool_headers = manifest(tc / "pi32v2/include")
    assert sha(REPLAY) == "f896e0fd8ce2719876271476db261002d0909392e8059158427d80118a3b396e"
    shutil.copy2(REPLAY, HERE / "native_fit_replay.py")
    write(HERE / "TOOLCHAIN-INPUTS.json", {"root": str(tc), "binaries": tool_pins, "include_headers": tool_headers,
                                          "clang_resource_files": manifest(tc / "common/lib/clang")})
    audit = {"status": "PREPARED_NOT_RUN", "canonical_source": str(source), "head": head, "git_tree": tree_id,
             "baseline": str(baseline), "selected_current_paths": selected, "changed_from_baseline": changed,
             "generated_inputs": GENERATED, "canonical_fastmath_sha256": sha(source / "firmware/src/dsp/fastmath.h"),
             "native_execution": "NOT_RUN", "emulator_execution": "NOT_RUN", "plans": []}
    env_before = dict(os.environ)
    for key in list(os.environ):
        if key.startswith(("X0X_", "FELUCCA_", "JIELI_")) or key in ("BOARD", "MODEL"):
            del os.environ[key]
    os.environ["JIELI_TOOLCHAIN"] = str(tc)
    sys.dont_write_bytecode = True
    originals = {k: getattr(subprocess, k) for k in ("Popen", "run", "check_output", "call", "check_call")}
    for key in originals:
        setattr(subprocess, key, forbidden)
    try:
        for model, profile, number in (("elite", "reference", "04"), ("elite", "shared", "05"), ("pro", "reference", "06"), ("pro", "shared", "07")):
            label = f"production-{profile}-{model}-{number}"
            run = HERE / "native-plans" / label
            run.mkdir(parents=True, exist_ok=False)
            tree = run / "tree"
            for relative in selected:
                dest = tree / relative
                dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(source / relative, dest)
            for name in GENERATED:
                dest = tree / "build/gen" / name
                dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(baseline / "build/gen" / name, dest)
            if profile == "reference":
                (tree / "firmware/src/dsp/fastmath.h").write_text(reference_header)
            sys.path.insert(0, str(tree / "tools"))
            for module in ("fm1pkg_make", "gen_cache", "lz4blk", "rom_guard", "check_original_inputs", "firmware_build_labels"):
                sys.modules.pop(module, None)
            spec = importlib.util.spec_from_file_location("production_fit_build", tree / "tools/build.py")
            build = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(build)
            build.BOARD, build.MODEL = "smk37", model
            build.fm1pkg_make.set_board("smk37")
            build.APP_SLOT, build.APP_CODE_MAX = build.fm1pkg_make.APP_SLOT, build.fm1pkg_make.APP_CODE_MAX
            build.OUT = tree / "build" / ("smk37-elite" if model == "elite" else "smk37")
            build.PRODUCT = build.fm1pkg_make.BOARDS["smk37"][2].replace(build.MODELS["pro"], build.MODELS[model])
            if model == "elite":
                build.BOARD_DEFS["smk37"].append('-DSMK_MODEL="SMK-37 Elite"')
            captured = []
            class Captured(Exception): pass
            def capture(*units):
                captured.extend(units)
                raise Captured()
            build.tc_all, build.tc = capture, forbidden
            cwd = Path.cwd()
            os.chdir(tree)
            try:
                try: build.build_app()
                except Captured: pass
            finally: os.chdir(cwd)
            assert len(captured) == 73, len(captured)
            commands, objects, source_units = [], [], []
            for unit in captured:
                assert unit[0] == "cc"
                argv = [str(v.relative_to(tree)) if isinstance(v, Path) and v.is_absolute() else str(v) for v in unit[1:]]
                src, obj = argv[argv.index("-c") + 1], argv[argv.index("-o") + 1]
                assert (tree / src).is_file(), src
                commands.append({"label": "compile-" + obj.replace("/", "_"), "argv": [str(tc / "pi32v2/bin/clang"), "-target", "pi32v2", *argv]})
                objects.append(obj)
                source_units.append(src)
            assert "firmware/src/dsp/juno6.c" in source_units
            assert not any("dnbx_core" in p or "dnbx_s1_envelope" in p for p in source_units)
            script = build.OUT / "app.ld"
            ldtext = (tree / "firmware/app.ld").read_text()
            assert ldtext.count("LENGTH = 0x8DFBC") == 1 and build.APP_CODE_MAX == 552672
            script.write_text(ldtext.replace("LENGTH = 0x8DFBC", "LENGTH = 0x86EE0"))
            elf, mapfile = run / "app.elf", run / "app.map"
            link = [str(tc / "pi32v2/bin/ld"), "-plugin", str(tc / "pi32v2/bin/LLVMgold.so"),
                    "-plugin-opt=-inline-threshold=0", "-plugin-opt=-inlinehint-threshold=0", "-plugin-opt=-inlinecold-threshold=0",
                    "-plugin-opt=-enable-tail-merge", "-plugin-opt=-tail-merge-size=1", "-plugin-opt=-tail-merge-threshold=10000",
                    "-plugin-opt=-function-sections", "-plugin-opt=-data-sections", "-plugin-opt=-fp-contract=off",
                    "--sort-section=alignment", "--gc-sections", "-e", "_start", "-T", str(script), "-Map", str(mapfile), *objects, "-o", str(elf)]
            plan = {"schema": 1, "status": "PREPARED_NOT_RUN", "created_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                    "input_tree": str(source), "tree": str(tree), "composition": "stock", "profile": profile, "model": model,
                    "source_commit": head, "source_git_tree": tree_id, "toolchain": str(tc),
                    "compiler_sha256": tool_pins["pi32v2/bin/clang"], "linker_sha256": tool_pins["pi32v2/bin/ld"],
                    "compile_commands": commands, "compile_source_paths": source_units, "link_command": link,
                    "link_script": str(script), "link_script_sha256": sha(script), "source_manifest": manifest(tree, True),
                    "all_input_manifest": manifest(tree), "elf": str(elf), "map": str(mapfile),
                    "physical_xip_capacity": 552672, "required_free_bytes": 1024, "native_execution": "NOT_RUN", "emulator_execution": "NOT_RUN",
                    "scope": "Current official stock app closure, default development _900 identity, five frozen generated headers; exactly six fastmath linkage declarations vary. No compact core, extra export roots, changed optimization flags, loader, packaging, target run, timing or quality acceptance."}
            write(run / "PLAN.json", plan)
            audit["plans"].append({"label": label, "path": str(run / "PLAN.json"), "sha256": sha(run / "PLAN.json"), "compile_units": len(commands), "input_files": len(plan["all_input_manifest"])})
    finally:
        for key, value in originals.items(): setattr(subprocess, key, value)
        os.environ.clear()
        os.environ.update(env_before)
    write(HERE / "PRODUCTION-PREPARATION.json", audit)
    print(json.dumps({"status": audit["status"], "head": head, "plans": audit["plans"]}, indent=2))

if __name__ == "__main__":
    main()
