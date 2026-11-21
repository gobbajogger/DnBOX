#!/usr/bin/env python3
"""Build and exercise the FWUI candidate on its real Sound provider, in a fresh directory."""
import argparse
import hashlib
import json
import os
import re
from pathlib import Path
import subprocess
import tarfile
import tempfile

BASE = "76040daa5b52d686a8b7b544296ba5d79610a95d"
SOUND = "80b7e1db59e5c3eceb2b3530e80c2cedf29c5b7f"
HERE = Path(__file__).resolve().parent


def git(repo, *args):
    return subprocess.check_output(["git", "-C", str(repo), *args])


def extract(repo, target, commit, paths):
    with tempfile.TemporaryFile() as archive:
        subprocess.run(["git", "-C", str(repo), "archive", commit, "--", *paths],
                       stdout=archive, check=True)
        archive.seek(0)
        with tarfile.open(fileobj=archive) as files:
            files.extractall(target, filter="data")


def compose(repo, target, fwui, test_ref):
    # Sound supplies the engine, simulator, stock assets and build definition.
    extract(repo, target, SOUND, ["assets/909", "assets/fonts", "firmware", "host",
            "tools", "experiments/original_synth", "experiments/wobble", "build.sh",
            "ble", "tests/host/original_build.sh", "tests/host/synthone_patch_test.c",
            "tests/host/synthone_patch_test.sh", "tests/host/synthone_init_audio.c",
            "tests/host/synthone_init_audio.sh"])
    # Three-way combine the FWUI changes to all shipping sources. A conflict
    # aborts before a binary can be presented as a result.
    with tempfile.TemporaryDirectory() as tmp:
        old, updated = Path(tmp) / "base", Path(tmp) / "fwui"
        names = git(repo, "diff", "--name-only", "-z", BASE, fwui, "--", "firmware", "host",
                    "tools", "assets", "ble", "build.sh", "experiments/original_synth",
                    "experiments/wobble")
        for name in names.decode().split("\0"):
            if not name:
                continue
            path = target / name
            tip = git(repo, "show", f"{fwui}:{name}")
            if not path.exists():
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(tip)
                continue
            base = git(repo, "show", f"{BASE}:{name}")
            if path.read_bytes() == base:
                path.write_bytes(tip)
                continue
            old.write_bytes(base)
            updated.write_bytes(tip)
            result = subprocess.check_output(["git", "merge-file", "-p", str(path),
                                              str(old), str(updated)])
            path.write_bytes(result)
    extract(repo, target, fwui, ["tests/host/smc_ui_test.c", "tests/host/smc_ui_test.sh"])
    extract(repo, target, test_ref, ["tests/host/ui_synthone_patch_test.c",
                                    "tests/host/ui_synthone_frame_test.c"])
    # Some installations keep these headers outside Git. Use only the loader's
    # authorized exact-size and SHA-256 inputs, and still run the standard verifier.
    manifest = json.loads((target / "experiments/original_synth/EXTERNAL-INPUTS.json").read_text())
    for entry in manifest["excluded_data_inputs"]:
        path = target / entry["path"]
        source = repo / entry["path"]
        if not path.is_file() and source.is_file():
            raw = source.read_bytes()
            if len(raw) != entry["bytes"] or hashlib.sha256(raw).hexdigest() != entry["sha256"]:
                raise ValueError(f"original input differs: {source}")
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(raw)
    subprocess.run(["python3", "tools/check_original_inputs.py"], cwd=target, check=True)


def fingerprint(board_dir, model):
    out = board_dir / ("smk37" if model == "pro" else "smk37-elite")
    package = "x0x-smk37.fwsc" if model == "pro" else "x0x-smk37-elite.fwsc"
    files = {"app": out / "x0x.bin", "elf": out / "x0x.elf", "loader": out / "loader/ota.bin",
             "package": board_dir / package}
    return {name: {"bytes": path.stat().st_size, "sha256": hashlib.sha256(path.read_bytes()).hexdigest()}
            for name, path in files.items()}


def build_boards(source, dest, reference):
    results = []
    for enabled in (0, 1):
        for model in ("elite", "pro"):
            log = dest / f"native-{model}-patch{enabled}.log"
            env = {k: v for k, v in os.environ.items() if not k.startswith("X0X_")}
            env.update(X0X_SYNTHONE_PATCH=str(enabled), X0X_JOBS="1")
            with log.open("w") as output:
                rc = subprocess.run(["sh", "build.sh", "--board", "smk37", "--model", model],
                                    cwd=source, env=env, stdout=output, stderr=subprocess.STDOUT).returncode
            result = {"model": model, "patch": enabled, "exit": rc, "log": log.name}
            contents = log.read_text()
            for key, caption in (("xip_free", "XIP free"), ("pool_margin", "pool margin"),
                                 ("pool_reserve", "pool reserve")):
                match = re.search(rf"^{caption}: (-?\d+) B$", contents, re.M)
                result[key] = int(match.group(1)) if match else None
            if rc == 0:
                result["binaries"] = fingerprint(source / "build", model)
                if reference is not None:
                    expected = reference["builds"][f"{model}-{enabled}"]
                    result["binary_match"] = result["binaries"] == expected["binaries"]
                    result["limits_match"] = all(result[k] == expected[k]
                                                 for k in ("xip_free", "pool_margin", "pool_reserve"))
            result["fit"] = (rc == 0 and result["xip_free"] is not None and result["xip_free"] >= 1024
                             and result["pool_reserve"] == 8192 and result["pool_margin"] is not None
                             and result["pool_margin"] >= 512)
            results.append(result)
            (dest / "native.json").write_text(json.dumps(results, indent=2) + "\n")
        # The next feature setting starts with new objects and headers.
        if (source / "build").exists():
            (source / "build").rename(dest / f"native-output-patch{enabled}")
    if not all(row["fit"] and row.get("binary_match", True) and row.get("limits_match", True)
               for row in results):
        raise SystemExit("Native fit or binary replay failed. See native.json and model logs.")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("out", type=Path, help="new directory for composed source and receipts")
    parser.add_argument("--fwui", default="HEAD", help="local FWUI commit containing the editor and test")
    parser.add_argument("--test-ref", default="HEAD", help="local revision of the two FWUI test fixtures")
    parser.add_argument("--native-receipt", type=Path,
                        help="require exact ELF, application, loader, package and memory replay from this receipt")
    parser.add_argument("--boards", action="store_true", help="also link stock Elite and Pro, patch off and on")
    parser.add_argument("--repo", type=Path, default=HERE.parents[1],
                        help="local Git repository containing all three pinned commits")
    args = parser.parse_args()
    dest = args.out.resolve()
    dest.mkdir(parents=True, exist_ok=False)
    source = dest / "source"
    source.mkdir()
    repo = args.repo.resolve()
    fwui = git(repo, "rev-parse", "--verify", args.fwui + "^{commit}").decode().strip()
    test_ref = git(repo, "rev-parse", "--verify", args.test_ref + "^{commit}").decode().strip()
    compose(repo, source, fwui, test_ref)
    refs = {"base": BASE, "fwui": fwui, "sound": SOUND,
            "test_sources": {p: git(repo,"rev-parse",f"{test_ref}:tests/host/{p}").decode().strip()
                             for p in ("ui_synthone_patch_test.c","ui_synthone_frame_test.c")}}
    (dest / "refs.json").write_text(json.dumps(refs, indent=2) + "\n")
    env = dict(os.environ, X0X_SYNTHONE_PATCH="1", X0X_HOST_DEFS="-DX0X_SYNTHONE_PATCH=1",
               SMC_UI_OUT=str(dest / "panel"), SMC_UI_TEST_FILE="tests/host/ui_synthone_patch_test.c")
    for name, command in [
        ("sound-provider", ["sh", "tests/host/synthone_patch_test.sh", str(dest / "provider")]),
        ("panel", ["sh", "tests/host/smc_ui_test.sh"]),
    ]:
        with (dest / (name + ".log")).open("w") as log:
            subprocess.run(command, cwd=source, env=env, stdout=log, stderr=subprocess.STDOUT,
                           check=True)
    frame_log = dest / "frame-review.log"
    frame_env = dict(env, SMC_UI_OUT=str(dest / "frame-review"),
                     SMC_UI_TEST_FILE="tests/host/ui_synthone_frame_test.c")
    with frame_log.open("w") as log:
        subprocess.run(["sh", "tests/host/smc_ui_test.sh"], cwd=source, env=frame_env,
                       stdout=log, stderr=subprocess.STDOUT, check=True)
    if "PASS Synth One F7 frame ownership and geometry" not in frame_log.read_text():
        raise ValueError(f"frame geometry marker missing in {frame_log}")
    generated_font = (source / "build/gen/felucca_font.h").read_text()
    for face, cap in (("T", (2, 9)), ("XS", (3, 11))):
        row = next(line for line in generated_font.splitlines() if f" FONT_{face} = " in line)
        if not row.endswith(f"{{ {cap[0]}, {cap[1]} }} }};"):
            raise ValueError(f"FWUI glyph cap metrics missing from {face}: {row}")
    pngs = sorted((dest / "panel").glob("bass-*.png"))
    assert len(pngs) == 88, len(pngs)
    from PIL import Image, ImageChops
    for path in pngs:
        with Image.open(path) as img:
            assert img.size == (240, 240), path
            if path.stem.endswith("-before"):
                other = path.with_name(path.name.replace("-before.png", "-applied.png"))
                with Image.open(other) as applied:
                    match = re.fullmatch(r"bass-[AB]-page-(\d+)-k(\d+)-before", path.stem)
                    page, knob = map(int, match.groups())
                    row_y, cell_x = 24 + 50 * (page - 1), 4 + 59 * (knob - 5)
                    # In an envelope row the selected knob's physical value is
                    # above the graph. Other controls put it below the glyph.
                    top, bottom = ((10, 23) if page >= 3 else (34, 48))
                    box = (cell_x, row_y + top, cell_x + 58, row_y + bottom)
                    assert ImageChops.difference(img.crop(box).convert("RGB"),
                                                 applied.crop(box).convert("RGB")).getbbox(), path
    # Exact R22 49px rows, 50px pitch, with the native font's cap bounds.
    # These blank scanlines disappeared in the label/curve collision.
    for owner in "AB":
        with Image.open(dest / "panel" / f"bass-{owner}-page-2-k0-overview.png") as img:
            for k in range(3):
                x = 4 + 59 * k
                for y in (74 + 21, 74 + 36):
                    assert len(img.crop((x+5, y, x+53, y+1)).getcolors()) == 1, (owner, k, y)
            for y in (124 + 10, 174 + 10):
                assert len(img.crop((12, y, 228, y+1)).getcolors()) == 1, (owner, y)
    manifest = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in pngs}
    (dest / "frames.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"{len(manifest)} captured 240x240 frames in {dest / 'panel'}")
    if args.boards or args.native_receipt:
        reference = json.loads(args.native_receipt.read_text()) if args.native_receipt else None
        build_boards(source, dest, reference)


if __name__ == "__main__":
    main()
