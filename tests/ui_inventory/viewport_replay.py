#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Replay indexed native LCD pixels in three real browser viewports, using the published emulator shell."""
import argparse
import base64
import csv
import hashlib
import io
import json
import os
import tarfile
import time
from pathlib import Path

from PIL import Image
from PyQt5.QtCore import QSize, Qt
from PyQt5.QtGui import QImage, QPainter
from PyQt5.QtWebKit import QWebSettings, qWebKitVersion
from PyQt5.QtWebKitWidgets import QWebPage
from PyQt5.QtWidgets import QApplication

PINS = {
    "index.html": "f71232cecb99b66ef3ae8512f047e27bde4ff4a7a70c40c49614db0b7d6861df",
    "style.css": "f9d8c09ebe689442f027b038482488a6e02db255db7553127b92b907b68cd5b8",
}
SIZES = [(1440, 900), (390, 844), (844, 390)]


def digest(data):
    return hashlib.sha256(data).hexdigest()


def shell_html(root):
    content = {}
    for name, expected in PINS.items():
        data = (root / name).read_bytes()
        if digest(data) != expected and data.endswith(b"\n"):
            data = data[:-1]
        if digest(data) != expected:
            raise ValueError(f"Unexpected published shell asset {name}")
        content[name] = data.decode()
    # Keep the DOM and stylesheet. The replay provides the firmware's captured LCD.
    return content["index.html"].replace('<link rel="stylesheet" href="style.css">',
        "<style>" + content["style.css"] + "</style>").replace('<script type="module" src="page.mjs"></script>', "")


def compact_png(data):
    # The archive deliberately stores uncompressed PNGs for xz cross-frame compression.
    image = Image.open(io.BytesIO(data)).convert("RGB")
    buf = io.BytesIO()
    image.save(buf, format="PNG", compress_level=1)
    return buf.getvalue()


def catalog(path):
    catalog_bytes = path.read_bytes()
    rows = list(csv.DictReader(io.StringIO(catalog_bytes.decode()), delimiter="\t"))
    print(f"Checking {len(rows)} indexed IDs", flush=True)
    archives = {}
    for name in {row["archive"] for row in rows if row["archive"]}:
        members = {}
        with tarfile.open(path.parent / name, "r|*") as archive:
            for item in archive:
                if item.name.endswith(".png"):
                    members[item.name] = members[item.linkname] if item.islnk() else compact_png(archive.extractfile(item).read())
        archives[name] = members
    frames = {}
    for row in rows:
        member, sha = row["png_path"], row["rgb_sha256"]
        if sha in frames:
            continue
        png = archives[row["archive"]][member] if row["archive"] else (path.parent / member).read_bytes()
        im = Image.open(io.BytesIO(png)).convert("RGB")
        if im.size != (240, 240) or digest(im.tobytes()) != sha:
            raise ValueError(f"Invalid native RGB for {row['state_id']}")
        frames[sha] = png if row["archive"] else compact_png(png)
    return rows, frames, digest(catalog_bytes)


def paint_frame(app, frame, png):
    data = base64.b64encode(png).decode()
    frame.evaluateJavaScript("""
        window.painted=false;
        (function () {
            var image=new Image();
            image.onload=function(){
                document.getElementById('lcd').getContext('2d').drawImage(image,0,0);
                window.painted=true;
            };
            image.src='data:image/png;base64,%s';
        }());
    """ % data)
    deadline = time.monotonic() + 10
    while not frame.evaluateJavaScript("window.painted"):
        app.processEvents()
        if time.monotonic() > deadline:
            raise RuntimeError("Browser did not decode the native PNG")


def bounds(frame):
    return frame.evaluateJavaScript("""(function(){
        var r=document.getElementById('lcd').getBoundingClientRect();
        return [r.left, r.top, r.width, r.height];
    }())""")


def raster(frame, x, y, width, height):
    image = QImage(width, height, QImage.Format_RGB32)
    painter = QPainter(image)
    painter.translate(-x, -y)
    frame.render(painter)
    painter.end()
    return image


def raw_rgb(image):
    rgb = image.convertToFormat(QImage.Format_RGB888)
    ptr = rgb.bits()
    ptr.setsize(rgb.byteCount())
    return bytes(ptr)


def replay(app, html, rows, frames, size, output):
    width, height = size
    page = QWebPage()
    page.setViewportSize(QSize(width, height))
    frame = page.mainFrame()
    for axis in [Qt.Horizontal, Qt.Vertical]:
        frame.setScrollBarPolicy(axis, Qt.ScrollBarAlwaysOff)
    frame.setHtml(html)
    app.processEvents()
    viewport = frame.evaluateJavaScript("[innerWidth,innerHeight]")
    initial = bounds(frame)
    if initial[1] + 240 > height:
        frame.evaluateJavaScript("document.getElementById('lcd').scrollIntoView(true)")
    visible = bounds(frame)
    x, y, w, h = visible
    if viewport != [width, height] or (w, h) != (240, 240) or x < 0 or y < 0 or x + w > width or y + h > height:
        raise ValueError(f"Clipped browser LCD at {size}: {visible}, {viewport}")
    if not frame.evaluateJavaScript("document.documentElement.scrollWidth <= innerWidth"):
        raise ValueError(f"Horizontal overflow at {size}")
    # CSS can produce subpixels. Qt snaps the painted box to the nearest device pixel.
    x, y = int(x + 0.5), int(y + 0.5)
    images = {row["rgb_sha256"]: row["state_id"] for row in rows
              if row["state_id"] in ("startup/startup-sample-recovered", "synth-one/bass-A-page-5-k0-overview",
                                    "superj6/slot-b-matrix-0")}
    print(f"Browser viewport {width}x{height}, {len(frames)} distinct images", flush=True)
    for sha, png in frames.items():
        paint_frame(app, frame, png)
        actual = raster(frame, x, y, 240, 240)
        if digest(raw_rgb(actual)) != sha:
            raise ValueError(f"LCD raster differs at {size}: {sha}")
        if sha in images:
            file = f"{width}x{height}-{images[sha].replace('/', '-')}.png"
            if not raster(frame, 0, 0, width, height).save(str(output / file)):
                raise OSError(file)
    return {"viewport": list(size), "initialLCD": initial, "visibleLCD": visible,
            "uniqueScreenshotsCompared": len(frames), "idComparisons": len(rows), "mismatches": 0}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("shell", type=Path)
    parser.add_argument("index", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    args.output.mkdir(exist_ok=True, parents=True)
    os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
    app = QApplication([])
    QWebSettings.setMaximumPagesInCache(0)
    QWebSettings.setObjectCacheCapacities(0, 0, 0)
    html = shell_html(args.shell)
    rows, frames, index_hash = catalog(args.index)
    measurements = [replay(app, html, rows, frames, size, args.output) for size in SIZES]
    result = {"browser": "Qt WebKit " + qWebKitVersion(), "shellSha256": PINS,
              "source": "Published emulator shell, native PNGs painted into its LCD canvas; script/audio disabled",
              "catalogSha256": index_hash, "ids": len(rows), "uniqueNativeRGB": len(frames),
              "viewportResults": measurements}
    (args.output / "viewport.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(measurements))


if __name__ == "__main__":
    main()
