#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Fail-closed local cross-repository UI report gate. No network or CI actions.

A report is evidence to verify, not authority to declare missing checks passed.
Individual producers own real render/DOM checks. This aggregator cannot replace
those checks, fresh human review or native/device qualification.
"""
from __future__ import annotations
import argparse
import hashlib
import io
import json
from pathlib import Path
import subprocess
import struct
import sys
import time
from PIL import Image

STATUSES = {"PASS", "FAIL", "REVIEW_REQUIRED", "NOT_RUN"}
WEB_IDS = set(range(10, 25))
FIRMWARE_IDS = set(range(1, 10)) | set(range(19, 25))
CANONICAL_SPEC_SHA256 = "868957f911acd3d2de67f61db41f2f49a552d0f023d69be67df96a637149add6"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def valid_hash(value: object) -> bool:
    return isinstance(value, str) and len(value) == 64 and all(c in "0123456789abcdef" for c in value)


def artifact_integrity(item: object, expected_source: str, root: Path | None = None,
                       screenshot: bool = False) -> list[str]:
    """Authenticate actual local artifact bytes, never merely an evidence label."""
    if not isinstance(item, dict) or not isinstance(item.get("path"), str) or not item["path"]:
        return ["artifact path is missing"]
    if not valid_hash(item.get("sha256")) or item.get("sourceClosureSha256") != expected_source:
        return ["artifact hash/source is missing or stale"]
    path = Path(item["path"])
    if not path.is_absolute() and root is not None:
        path = root / path
    try:
        path = path.resolve(strict=True)
        if root is not None and not path.is_relative_to(root.resolve()):
            return ["artifact is outside its declared evidence directory"]
        if not path.is_file():
            return ["artifact is not a regular file"]
        data = path.read_bytes()
    except (OSError, RuntimeError) as exc:
        return [f"artifact is unavailable: {type(exc).__name__}"]
    if hashlib.sha256(data).hexdigest() != item["sha256"]:
        return ["artifact bytes do not match SHA256"]
    if screenshot:
        if len(data) < 24 or data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
            return ["screen artifact is not a PNG with dimensions"]
        dimensions = struct.unpack(">II", data[16:24])
        if dimensions != (item.get("width"), item.get("height")) or min(dimensions) <= 0:
            return ["screen dimensions do not match actual PNG"]
        try:
            with Image.open(io.BytesIO(data)) as decoded:
                if decoded.format != "PNG" or decoded.size != dimensions:
                    return ["screen is not the declared PNG image"]
                decoded.verify()
            with Image.open(io.BytesIO(data)) as decoded:
                decoded.load()
        except (OSError, SyntaxError, ValueError, Image.DecompressionBombError):
            return ["screen PNG bytes fail complete image decoding"]
    elif not isinstance(item.get("kind"), str) or not item["kind"]:
        return ["artifact kind is missing"]
    return []


def inspect_report(report: dict, expected_source: str, required: set[int], scope: str,
                   artifact_root: Path | None = None) -> list[str]:
    problems = []
    if report.get("canonicalSpecSHA256") != CANONICAL_SPEC_SHA256:
        problems.append(f"{scope}: exact canonical owner-spec binding is missing or stale")
    if not valid_hash(expected_source) or report.get("sourceClosureSha256") != expected_source:
        problems.append(f"{scope}: source closure is missing or does not match")
    checks = report.get("checks", [])
    if not isinstance(checks, list):
        return problems + [f"{scope}: checks is not a list"]
    indexed = {}
    for check in checks:
        if not isinstance(check, dict) or not isinstance(check.get("id"), int):
            problems.append(f"{scope}: malformed check")
            continue
        cid = check["id"]
        if cid in indexed:
            problems.append(f"{scope}: duplicate check {cid}")
        indexed[cid] = check
        if check.get("status") not in STATUSES:
            problems.append(f"{scope}: check {cid} has unknown status")
        if check.get("status") == "PASS" and not check.get("evidence"):
            problems.append(f"{scope}: check {cid} PASS has no evidence")
        if check.get("status") == "PASS":
            for evidence in check.get("evidence", []):
                for reason in artifact_integrity(evidence, expected_source, artifact_root):
                    problems.append(f"{scope}: check {cid}: {reason}")
    for cid in sorted(required):
        check = indexed.get(cid)
        if check is None:
            problems.append(f"{scope}: required check {cid} is missing")
        elif check.get("status") != "PASS":
            problems.append(f"{scope}: required check {cid} is {check.get('status')}")
    for negative in report.get("negatives", []):
        if not negative.get("defectId") or negative.get("oldExit") in (None, 0):
            problems.append(f"{scope}: old-defect fixture did not demonstrate failure")
        if not valid_hash(negative.get("oldSourceClosureSha256")):
            problems.append(f"{scope}: old-defect source is unpinned")
        if negative.get("currentSourceClosureSha256") != expected_source or negative.get("currentExit") != 0:
            problems.append(f"{scope}: current negative-control repair is unverified")
        if not negative.get("evidence"):
            problems.append(f"{scope}: old/current negative-control artifacts are missing")
        for evidence in negative.get("evidence", []):
            for reason in artifact_integrity(evidence, expected_source, artifact_root):
                problems.append(f"{scope}: negative {negative.get('defectId')}: {reason}")
    defects = {item.get("defectId") for item in report.get("negatives", [])}
    for item in report.get("humanDefects", []):
        if item.get("id") not in defects:
            problems.append(f"{scope}: human defect {item.get('id')} lacks its real regression fixture")
    screens = report.get("screens", [])
    if not screens:
        problems.append(f"{scope}: no per-screen human review")
    applicable = [s for s in screens if s.get("applicability") != "NOT_APPLICABLE"]
    if not applicable:
        problems.append(f"{scope}: no applicable rendered/reviewed screen")
    inventory = report.get("coverageInventory", [])
    if not isinstance(inventory, list) or not inventory:
        problems.append(f"{scope}: explicit screen/state coverage inventory is missing")
    else:
        ids = [s.get("id") for s in screens]
        expected = [s.get("id") for s in inventory if isinstance(s, dict)]
        if len(expected) != len(inventory) or len(set(expected)) != len(expected) or set(expected) != set(ids) or len(set(ids)) != len(ids):
            problems.append(f"{scope}: reviewed screens do not match explicit coverage inventory")
    for screen in screens:
        if screen.get("applicability") == "NOT_APPLICABLE":
            if not screen.get("reason"):
                problems.append(f"{scope}: unsupported screen lacks reason")
            continue
        if screen.get("sourceClosureSha256") != expected_source:
            problems.append(f"{scope}: screen {screen.get('id')} review is stale")
        for reason in artifact_integrity(screen, expected_source, artifact_root, screenshot=True):
            problems.append(f"{scope}: screen {screen.get('id')}: {reason}")
        if screen.get("score") != 10 or screen.get("defects"):
            problems.append(f"{scope}: screen {screen.get('id')} is not accepted")
    if report.get("coverageComplete") is not True:
        problems.append(f"{scope}: all-screen/state coverage is not established")
    run = report.get("run", {})
    if run.get("exit") != 0 or run.get("firstAttempt") is not True:
        problems.append(f"{scope}: no clean first-run receipt")
    if run.get("sourceClosureSha256") != expected_source:
        problems.append(f"{scope}: run receipt source is stale")
    return problems


def verify_pushed_revision(report: dict, receipt: dict, scope: str) -> list[str]:
    """The owner supplies a saved connector readback; no network Git is used."""
    rev = report.get("revision")
    if not isinstance(rev, str) or len(rev) != 40:
        return [f"{scope}: no exact committed revision"]
    actual = receipt.get("object", {}).get("sha") or receipt.get("head", {}).get("sha")
    if actual != rev:
        return [f"{scope}: pushed ref readback does not match reviewed revision"]
    return []


def record_run(args: argparse.Namespace) -> int:
    """Append every actual attempt, keyed by declared source and input closure.

    Old-defect/mutant runs use different source/input identities. A failed first
    current-input run followed by a pass stays a failed first-run gate.
    """
    ledger = Path(args.ledger)
    ledger.parent.mkdir(parents=True, exist_ok=True)
    records = [json.loads(line) for line in ledger.read_text().splitlines()] if ledger.exists() else []
    identity = (args.source, args.inputs, args.case)
    previous = [r for r in records if (r["source"], r["inputs"], r["case"]) == identity]
    row = {"source": args.source, "inputs": args.inputs, "case": args.case,
           "attempt": len(previous) + 1, "command": args.command, "started": time.time()}
    if not valid_hash(args.source) or not valid_hash(args.inputs) or not args.command:
        raise ValueError("source/input hashes and an actual command are required")
    result = subprocess.run(args.command, check=False)
    row.update(exit=result.returncode, finished=time.time())
    with ledger.open("a") as stream:
        stream.write(json.dumps(row, sort_keys=True) + "\n")
    first = previous[0] if previous else row
    if first["exit"] != 0:
        print("FIRST_RUN_FAIL: prior failure retained; rerun is not a clean pass", file=sys.stderr)
        return 1
    return result.returncode


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subs = parser.add_subparsers(dest="action", required=True)
    run = subs.add_parser("record-run")
    for name in ("source", "inputs", "case", "ledger"):
        run.add_argument("--" + name, required=True)
    run.add_argument("command", nargs=argparse.REMAINDER)
    check = subs.add_parser("verify")
    check.add_argument("--canonical-spec", type=Path,
                       default=Path(__file__).with_name("canonical-spec.json"))
    for scope in ("firmware", "web"):
        check.add_argument("--" + scope + "-report", required=True, type=Path)
        check.add_argument("--" + scope + "-source", required=True)
        check.add_argument("--" + scope + "-pushed-receipt", required=True, type=Path)
    args = parser.parse_args()
    if args.action == "record-run":
        if args.command[:1] == ["--"]:
            args.command.pop(0)
        return record_run(args)
    problems = []
    from canonical_spec import authenticate, read_descriptor
    try:
        descriptor = read_descriptor(args.canonical_spec)
        actual_spec = authenticate(Path(__file__).resolve().parents[2], descriptor)
        if actual_spec != CANONICAL_SPEC_SHA256:
            problems.append("canonical owner document differs from reviewed gate pin")
    except (OSError, ValueError, KeyError, TypeError) as exc:
        problems.append(f"canonical owner document cannot be authenticated: {type(exc).__name__}")
    for scope, required in (("firmware", FIRMWARE_IDS), ("web", WEB_IDS)):
        report = json.loads(getattr(args, scope + "_report").read_text())
        receipt = json.loads(getattr(args, scope + "_pushed_receipt").read_text())
        problems += inspect_report(report, getattr(args, scope + "_source"), required, scope,
                                   getattr(args, scope + "_report").parent)
        problems += verify_pushed_revision(report, receipt, scope)
    for problem in problems:
        print("BLOCK: " + problem)
    if problems:
        print(f"UI DELIVERY GATE FAIL: {len(problems)} unresolved contracts")
        return 1
    print("UI DELIVERY GATE PASS: exact reviewed sources and pushed readbacks match")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
