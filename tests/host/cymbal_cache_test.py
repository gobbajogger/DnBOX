#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Reusing equal-length, equal-mtime edits to encoded cymbals must miss both builds."""
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import gen_cache


def main():
    with tempfile.TemporaryDirectory() as temporary:
        root = Path(temporary)
        shutil.copytree(ROOT / "assets/909", root / "assets/909")
        # Use the real host entry point and generator, stopping at the break-bank step.
        (root / "host").mkdir()
        (root / "tools").mkdir()
        shutil.copy(ROOT / "host/build_host.sh", root / "host")
        shutil.copy(ROOT / "tools/gen_drum_samples.py", root / "tools")
        (root / "tools/gen_builtin_break.sh").write_text("exit 0\n")
        gen = root / "build/gen"
        gen.mkdir(parents=True)
        (gen / "felucca_font.h").touch()
        host = ["sh", "host/build_host.sh", "--generate-only"]
        subprocess.run(host, cwd=root, check=True)
        with patch.object(gen_cache, "SRC", root), patch.dict(os.environ, {"X0X_GEN_CACHE": str(root / "cache")}):
            (gen / "x0x_break_bank.h").write_text("fixture")
            (gen / "felucca_font.h").write_text("fixture")
            gen_cache.store(gen)
            assert gen_cache.restore(root / "copy")
            for path in sorted((root / "assets/909/encoded").iterdir()):
                data = path.read_bytes()
                stamp = path.stat()
                modified = bytearray(data)
                # Keep the manifest syntactically valid and change just one digest digit.
                index = data.index(b'adpcm_sha256": "') + len(b'adpcm_sha256": "') if path.suffix == ".json" else 0
                modified[index] = ord("1") if data[index] == ord("0") else ord("0")
                path.write_bytes(modified)
                os.utime(path, ns=(stamp.st_atime_ns, stamp.st_mtime_ns))
                assert not gen_cache.restore(root / "copy"), f"stale native cache accepted {path.name}"
                result = subprocess.run(host, cwd=root, capture_output=True, text=True)
                assert result.returncode and "encoded data differs" in result.stderr, path.name
                path.write_bytes(data)
                assert gen_cache.restore(root / "copy")
    print("PASS four same-length, same-mtime mutations miss both build entry points")


if __name__ == "__main__":
    main()
