#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Verify the packed screenshots and export a searchable case-to-archive index."""
import argparse
import csv
import hashlib
import io
import json
import os
import tarfile
from pathlib import Path

from PIL import Image


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def apply_replacements(directory, index, rows):
    by_id = {row[0]: row for row in rows}
    for part in index.get("replacementArchives", []):
        path = directory / part["path"]
        if path.stat().st_size != part["bytes"] or sha256(path.read_bytes()) != part["sha256"]:
            raise ValueError(f"Replacement archive integrity failure in {path.name}")
        with tarfile.open(path, "r:xz") as archive:
            receipt = json.load(archive.extractfile("receipt.json"))
            updated = set()
            files = {"receipt.json"}
            for frame in receipt["frames"]:
                state_id = frame["id"]
                previous = by_id[state_id]
                if state_id in updated or previous[4] != frame["previousRGB"]:
                    raise ValueError(f"Invalid predecessor for {state_id}")
                name = frame["png"]
                if name in files:
                    raise ValueError(f"Repeated replacement image {name}")
                data = archive.extractfile(name).read()
                with Image.open(io.BytesIO(data)) as im:
                    digest = sha256(im.convert("RGB").tobytes())
                    if im.format != "PNG" or im.size != (240, 240) or digest != frame["rgbSHA256"]:
                        raise ValueError(f"Invalid replacement image {name}")
                by_id[state_id] = (state_id, path.name, name, previous[3], digest)
                updated.add(state_id)
                files.add(name)
            if len(updated) != part["frames"] or sorted(files) != sorted(archive.getnames()):
                raise ValueError(f"Replacement archive membership mismatch in {path.name}")
    return by_id.values()


def archived_rows(directory):
    index = json.loads((directory / "index.json").read_text())
    frames = None
    members = {}
    for part in index["archives"]:
        path = directory / part["path"]
        if path.stat().st_size != part["bytes"] or sha256(path.read_bytes()) != part["sha256"]:
            raise ValueError(f"Archive integrity failure in {path.name}")
        seen = {}
        with tarfile.open(path, "r|xz") as archive:
            for item in archive:
                if item.name == "frame-map.json":
                    if frames is not None:
                        raise ValueError("Duplicate frame map")
                    frames = json.load(archive.extractfile(item))
                if not item.name.startswith("renders/"):
                    continue
                if item.name in members:
                    raise ValueError(f"Duplicate screenshot {item.name}")
                if item.islnk():
                    digests = seen[item.linkname]
                elif item.isfile():
                    png = archive.extractfile(item).read()
                    with Image.open(io.BytesIO(png)) as im:
                        if im.format != "PNG" or im.size != (240, 240):
                            raise ValueError(f"Wrong screenshot format for {item.name}")
                        digests = sha256(png), sha256(im.convert("RGB").tobytes())
                else:
                    raise ValueError(f"Unexpected archive member {item.name}")
                members[item.name] = (path.name, digests)
                seen[item.name] = digests
    if frames is None or len(members) != index["frames"] or len(frames) != len(members):
        raise ValueError("Missing screenshots or frame map")
    rows = []
    for frame in frames:
        name = frame["storedPath"]
        part, digests = members.pop(name)
        if digests != (frame["storedPNGsha256"], frame["pixelSha256"]):
            raise ValueError(f"Screenshot digest mismatch for {name}")
        rows.append((frame["id"], part, name, frame["evidenceKind"], digests[1]))
    if members or len({r[0] for r in rows}) != index["stableStateIDs"]:
        raise ValueError("Inconsistent state or screenshot count")
    current = {row[0]: row for row in apply_replacements(directory, index, rows)}
    for name, previous in index.get("retiredStates", {}).items():
        _, archive, member, scope, digest = current.pop(name)
        if previous != digest:
            raise ValueError(f"Retired screen predecessor differs: {name}")
        historical = "history/" + name
        current[historical] = (historical, archive, member, "retired / " + scope, digest)
    return sorted(current.values())


def image_rgb(source, name):
    with Image.open(source) as image:
        if image.format != "PNG" or image.size != (240, 240):
            raise ValueError(f"Expected a 240x240 PNG: {name}")
        return image.convert("RGB")


def native_rows(screens, base):
    directory = screens.parent / "current/native-routes"
    for group, folder, receipts in [
        ("native", directory, ["report.json", "wave-frame.json"]),
        ("startup", directory / "normal-loop", ["normal-loop.json"]),
    ]:
        expected = {}
        for receipt in receipts:
            expected.update(json.loads((folder / receipt).read_text())["rgbSha256"])
        with (folder / "screen-index.tsv").open() as stream:
            for row in csv.DictReader(stream, delimiter="\t"):
                name = row["state_id"]
                path = os.path.relpath(folder / row["file"], screens)
                member = row["png_path_in_archive"]
                if member:
                    digest = base[path, member]
                else:
                    digest = sha256(image_rgb(screens / path, name).tobytes())
                if expected.pop(name) != digest or row["rgb_sha256"] != digest:
                    raise ValueError(f"Native route differs: {name}")
                yield (group + "/" + name, path if member else "", member or path, row["scope"], digest)
        if expected:
            raise ValueError(f"Missing {group} routes: {sorted(expected)}")

    # These target captures have a different RGB565 expansion than the host.
    # Verify the target oracle in its own color space and index actual PNG RGB.
    from stock_panel_routes import LUT_5, LUT_6
    guard = json.loads((directory / "stock-guard.json").read_text())
    for model, run in guard["models"].items():
        for shot, case in run["shots"].items():
            for label, result in case["firstMatchingFrames"].items():
                name = f"{model}-{shot}-{label}"
                path = directory / "guard" / (name + ".png")
                rgb = image_rgb(path, name)
                if sha256(rgb.point(LUT_5 + LUT_6 + LUT_5).tobytes()) != result["canonicalRgbSha256"]:
                    raise ValueError(f"Target guard frame differs: {name}")
                yield ("stock/" + name, "", os.path.relpath(path, screens),
                       "linked-stock/emu360", sha256(rgb.tobytes()))
    stock = json.loads((directory / "stock-final.json").read_text())
    for name, case in [("stock-splash", "recover-splash"), ("stock-ordinary", "recover")]:
        path = directory / (name + ".png")
        digest = sha256(image_rgb(path, name).tobytes())
        if any(model["startup"]["cases"][case]["rgbSha256"] != digest for model in stock["models"].values()):
            raise ValueError(f"Target startup frame differs: {name}")
        yield ("stock/" + name, "", os.path.relpath(path, screens), "linked-stock/emu360", digest)


def packed_routes(screens, archive_path, manifest, group, scope):
    with tarfile.open(archive_path) as archive:
        if sorted(archive.getnames()) != sorted(manifest):
            raise ValueError(f"Frame membership differs: {archive_path.name}")
        for member in archive:
            if not member.isfile():
                raise ValueError(f"Expected a regular PNG: {member.name}")
            data = archive.extractfile(member).read()
            if sha256(data) != manifest[member.name]:
                raise ValueError(f"Frame bytes differ: {member.name}")
            rgb = image_rgb(io.BytesIO(data), member.name)
            yield (group + "/" + Path(member.name).stem, os.path.relpath(archive_path, screens),
                   member.name, scope, sha256(rgb.tobytes()))


def synthone_rows(screens):
    directory = screens.parent / "current/synthone-editor"
    scope = "Sound #973 / X0X_SYNTHONE_PATCH=1 / INIT"
    qualification = json.loads((directory / "qualification.json").read_text())
    for name, key in [("frames.tar.gz", "archive_sha256"), ("frames.json", "frames_manifest_sha256")]:
        if sha256((directory / name).read_bytes()) != qualification[key]:
            raise ValueError(f"Original Synth One {name} differs")
    matrix = json.loads((directory / "frames.json").read_text())
    rows = {r[0]: r for r in packed_routes(screens, directory / "frames.tar.gz", matrix,
                                            "synth-one", scope)}
    # Every original matrix ID must survive the layout change.
    replay = json.loads((directory / "routes-qualification.json").read_text())
    for file_field, hash_field in [("archive_file", "archive_sha256"), ("manifest_file", "manifest_sha256")]:
        if sha256((directory / replay[file_field]).read_bytes()) != replay[hash_field]:
            raise ValueError(f"Current Synth One {replay[file_field]} differs")
    routes = json.loads((directory / "routes.json").read_text())
    ids = (2, 5, 16, 17, 19, 22, 23, 24, 25, 26, 27, 28, 29)
    lists = {f"list-{owner}-factory-id-{control:02d}.png" for owner in "AB" for control in ids}
    lists.update(f"list-{owner}-last.png" for owner in "AB")
    live = {"bass-a-attack.png", "bass-a-held.png", "bass-a-released.png",
            "bass-b-idle-control.png", "bass-b-held-control.png", "simultaneous-editor-busy.png"}
    if set(routes) != set(matrix) | lists | live:
        raise ValueError("Incomplete Synth One matrix, parameter list or live route replay")
    for row in packed_routes(screens, directory / "routes.tar.xz", routes, "synth-one", scope):
        rows[row[0]] = row
    return rows.values()


def stock_init_rows(screens):
    directory = screens.parent / "current/panel-routes/stock-init"
    receipt = json.loads((directory / "frames.json").read_text())
    from stock_init_frames import frame_names
    names = frame_names()
    if set(receipt) != names:
        raise ValueError("Incomplete stock INIT selector capture")
    for name, expected in receipt.items():
        path = directory / (name + ".png")
        digest = sha256(image_rgb(path, name).tobytes())
        if digest != expected:
            raise ValueError(f"Stock INIT selector pixels differ: {name}")
        yield ("stock-init/" + name.removeprefix("stock-init-"), "", os.path.relpath(path, screens),
               "stock / X0X_SYNTHONE_PATCH=0 / SMK-37 host", digest)


def superj6_rows(screens):
    directory = screens.parent / "current/superj6-slot"
    receipt = json.loads((directory / "frames.json").read_text())
    if sha256((directory / "frames.tar.gz").read_bytes()) != receipt["archive_sha256"]:
        raise ValueError("Compact SuperJ6 archive differs")
    frames = receipt["frames"]
    by_name = {frame["file"]: frame for frame in frames}
    pages = ("oscillator", "filter", "envelope", "lfo-chorus")
    expected = {f"slot-{owner}-{route}-{state}.png"
                for owner in "ab"
                for route, states in (("matrix", pages), ("list", (*pages, "sound", "last")),
                                      ("sound", ("poly", "pad", "pluck", "brass")))
                for state in states}
    if len(by_name) != len(frames) or set(by_name) != expected:
        raise ValueError("Missing or duplicate SuperJ6 route captures")
    manifest = {name: frame["sha256"] for name, frame in by_name.items()}
    for row in packed_routes(screens, directory / "frames.tar.gz", manifest, "superj6",
                             "stock / compact SuperJ6"):
        frame = by_name[row[2]]
        if row[4] != frame["rgb_sha256"]:
            raise ValueError(f"SuperJ6 framebuffer differs: {row[2]}")
        yield row


def build_index(directory, output, check=False):
    rows = archived_rows(directory)
    base = {(archive, member): digest for _, archive, member, _, digest in rows}
    rows.extend(native_rows(directory, base))
    rows.extend(stock_init_rows(directory))
    rows.extend(synthone_rows(directory))
    rows.extend(superj6_rows(directory))
    if len({row[0] for row in rows}) != len(rows):
        raise ValueError("Duplicate screen state ID")
    stream = io.StringIO(newline="")
    writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
    writer.writerow(("state_id", "archive", "png_path", "evidence_kind", "rgb_sha256"))
    writer.writerows(sorted(rows))
    data = stream.getvalue().encode()
    if check:
        if data != output.read_bytes():
            raise ValueError(f"Screen index is stale: {output}")
    else:
        output.write_bytes(data)
    print(f"Verified {len(rows)} indexed 240x240 captures")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--check", action="store_true", help="verify the committed index without rewriting it")
    args = parser.parse_args()
    build_index(args.directory, args.output, args.check)
