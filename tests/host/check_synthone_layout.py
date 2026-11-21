#!/usr/bin/env python3
"""Judge R22 A3 on native Synth One framebuffers using the visible control ink."""
import argparse
import io
import json
from pathlib import Path
import sys
import tarfile
from collections import Counter

from PIL import Image, ImageChops

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "ui_defects/first_slice"))
from detectors import largest_empty_rectangle

FRAME_PIXELS = 240 * 240
# RGB565 dark-theme outlines after the host PNG writer expands them to RGB888.
# The brighter of the outline, face and divider colors is the outline in both
# selected states. Excluding this layer prevents a bare rounded slab from passing.
OUTLINES = {(32, 56, 57), (57, 93, 65)}
BODY = ["oscillator", "filter", "filter-envelope", "amp-envelope", "sound"]


def editor_cases(directory):
    for owner in "AB":
        for page, count in enumerate([2, 3, 4, 4, 0], 1):
            rows = BODY[:4] if page < 5 else BODY[1:]
            boxes = [(title, 24 + 50 * row, 49) for row, title in enumerate(rows)]
            prefix = f"bass-{owner}-page-{page}-k"
            yield directory / f"{prefix}0-overview.png", boxes
            for col in range(5, 5 + count):
                for event in ["before", "touched", "applied"]:
                    yield directory / f"{prefix}{col}-{event}.png", boxes


def stock_cases(directory):
    for owner in "ab":
        for state in ["ready", "updating", "pending", "long-name",
                      "list-engine", "list-preset", "list-status"]:
            boxes = [(title, 24 + 67 * row, 66)
                     for row, title in enumerate(["engine", "preset", "status"])]
            yield directory / f"stock-init-{state}-{owner}.png", boxes


def editor_lists(directory):
    ids = [2, 5, 16, 17, 19, 22, 23, 24, 25, 26, 27, 28, 29]
    for owner in "AB":
        for row, factory_id in enumerate(ids):
            yield directory / f"list-{owner}-factory-id-{factory_id:02d}.png", min(row, 8)
        yield directory / f"list-{owner}-last.png", 8


def juno_cases(directory):
    groups = ["oscillator", "filter", "envelope", "lfo-chorus"]
    for owner in "ab":
        for group in groups:
            yield directory / f"slot-{owner}-matrix-{group}.png", [
                (name, 24 + 50 * row, 49) for row, name in enumerate(groups)]
        for name in ["poly", "pad", "pluck", "brass"]:
            yield directory / f"slot-{owner}-sound-{name}.png", [
                (title, 24 + 50 * row, 49)
                for row, title in enumerate(groups[1:] + ["sound"])]
        for row, name in [(0, "oscillator"), (4, "filter"), (8, "envelope"),
                          (8, "lfo-chorus"), (8, "sound"), (8, "last")]:
            yield directory / f"slot-{owner}-list-{name}.png", row


def measure_ink(ink, title):
    width, inner_height = ink.size
    raw = ink.tobytes()
    mask = [raw[i:i + width] for i in range(0, len(raw), width)]
    gap = largest_empty_rectangle(mask)
    bbox = ink.getbbox()
    area = (bbox[2] - bbox[0]) * (bbox[3] - bbox[1]) if bbox else 0
    good = 100 * area >= 70 * width * inner_height and 100 * gap["area"] <= 12 * FRAME_PIXELS
    return {"row": title, "status": "PASS" if good else "FAIL",
            "inkBoundsInPanel": bbox, "inkBoundsArea": area,
            "panelPixels": width * inner_height, "largestEmptyPixels": gap["area"],
            "coveragePercent": round(100 * area / (width * inner_height), 3),
            "emptyScreenPercent": round(100 * gap["area"] / FRAME_PIXELS, 3)}


def measure_panel(image, title, top, height):
    outline = image.getpixel((120, top))
    if outline not in OUTLINES:
        return {"row": title, "status": "FAIL", "reason": "missing R22 panel outline"}
    interior = image.crop((4, top + 1, 236, top + height - 1))
    red, green, blue = interior.split()
    brightest = ImageChops.lighter(ImageChops.lighter(red, green), blue)
    ink = brightest.point(lambda x: 255 if x > max(outline) else 0)
    return measure_ink(ink, title)


def list_row_ink(image, y, height=19):
    # Each rounded interactive row has a 227px interior with its label,
    # value, unit and optional meter. Short menus may have taller rows.
    # Never count the tint or antialiased shoulders of the row as content.
    ink = Image.new("L", (227, height))
    for dy in range(2, height + 2):
        left, right = (12, 224) if dy in [2, 3, height, height + 1] else (8, 227)
        scan = [image.getpixel((x, y + dy)) for x in range(left, right)]
        face = Counter(scan).most_common(1)[0][0]
        for x, pixel in enumerate(scan, left):
            if pixel != face:
                ink.putpixel((x - 4, dy - 2), 255)
    return ink


def measure_list(image, selected):
    # The scrolling surface must fill the LCD, each individual value slab
    # must satisfy panel occupancy, and the physical selection must be visible.
    panels = []
    selection = []
    for row in range(9):
        y = 20 + 23 * row
        if max(image.getpixel((120, y + 1))) > 110:
            selection.append(row)
        ink = list_row_ink(image, y)
        panel = measure_ink(ink, f"list-row-{row + 1}")
        if not ink.crop((4, 0, 98, 19)).getbbox():
            panel["status"] = "FAIL"
        panels.append(panel)
    longest = gap = 0
    for y in range(20, 240):
        scan = image.crop((0, y, 240, y + 1))
        is_flat = not ImageChops.difference(scan, Image.new("RGB", scan.size, scan.getpixel((0, 0)))).getbbox()
        gap = gap + 1 if is_flat else 0
        longest = max(longest, gap)
    panels.append({"row": "list-window", "selectedRows": selection, "uniformScanlines": longest,
                   "emptyScreenPercent": round(100 * longest / 240, 3),
                   "status": "PASS" if selection == [selected] and 100 * longest <= 12 * 240 else "FAIL"})
    return panels


def judge(path, boxes, content=None):
    with Image.open(content or path) as source:
        if source.format != "PNG" or source.size != (240, 240):
            raise ValueError(f"Expected a native 240x240 PNG at {path}")
        image = source.convert("RGB")
    panels = (measure_list(image, boxes) if isinstance(boxes, int)
              else [measure_panel(image, *box) for box in boxes])
    return {"frame": path.name, "panels": panels,
            "status": "PASS" if all(p["status"] == "PASS" for p in panels) else "FAIL"}


def archived_cases(archive_path, cases):
    with tarfile.open(archive_path) as archive:
        for path, boxes in cases:
            member = archive.getmember(path.name)
            if not member.isfile():
                raise ValueError(f"Expected a PNG file in {archive_path}: {path.name}")
            with archive.extractfile(member) as payload:
                yield judge(path, boxes, io.BytesIO(payload.read()))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--editor", type=Path, help="host captures from the patch-enabled panel")
    parser.add_argument("--stock", type=Path, help="host captures from the stock INIT selector")
    parser.add_argument("--editor-lists", type=Path, help="optional Synth One physical list captures")
    parser.add_argument("--juno", type=Path, help="compact SuperJ6 matrix and physical list captures")
    parser.add_argument("--catalog", type=Path, help="committed docs/lanes/fwui/screens directory")
    parser.add_argument("--output", type=Path, help="write per-frame measurements as JSON")
    args = parser.parse_args()
    if not any([args.editor, args.editor_lists, args.stock, args.juno, args.catalog]):
        parser.error("provide a framebuffer directory or --catalog")
    cases = []
    if args.editor:
        cases.extend(editor_cases(args.editor))
    if args.stock:
        cases.extend(stock_cases(args.stock))
    if args.editor_lists:
        cases.extend(editor_lists(args.editor_lists))
    if args.juno:
        cases.extend(juno_cases(args.juno))
    if args.catalog:
        current = args.catalog.parent / "current"
        cases.extend(stock_cases(current / "panel-routes/stock-init"))
    results = [judge(path, boxes) for path, boxes in cases]
    if args.catalog:
        editor = current / "synthone-editor"
        results.extend(archived_cases(editor / "routes.tar.xz",
                                      [*editor_cases(editor), *editor_lists(editor)]))
        juno = current / "superj6-slot"
        results.extend(archived_cases(juno / "frames.tar.gz", juno_cases(juno)))
    failures = [r for r in results if r["status"] == "FAIL"]
    if args.output:
        args.output.write_text(json.dumps({"frames": results}, indent=2) + "\n")
    for r in failures[:4]:
        print(r["frame"], [p for p in r["panels"] if p["status"] == "FAIL"])
    print(f"R22 Synth One A3: {len(results)} frames, {sum(len(r['panels']) for r in results)} panels, "
          f"{len(failures)} failing frames")
    return bool(failures)


if __name__ == "__main__":
    sys.exit(main())
