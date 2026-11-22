#!/usr/bin/env python3
"""Measure over-budget scratch ELF sections. This runner never emits an app or package."""
import json
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path.cwd() / "tools"))
import build

native_tool = build.tc


def measure(tool, *args):
    if tool != "pi32v2/bin/ld" or "_start" not in args:
        return native_tool(tool, *args)

    link = list(args)
    script_index = link.index("-T") + 1
    stock_script = Path(link[script_index])
    if stock_script.name != "app.ld":
        return native_tool(tool, *args)
    text = stock_script.read_text()
    region = re.compile(r"(XIP\s+\(rx\)\s+:[^\n]*LENGTH\s*=\s*)0x[0-9A-Fa-f]+")
    widened, count = region.subn(
        lambda match: f"{match[1]}{build.APP_CODE_MAX + 65536:#x}", text
    )
    if count != 1:
        raise SystemExit("Expected one XIP region in the stock linker script")

    scratch = build.OUT / "xip-diagnostic.ld"
    elf = build.OUT / "xip-diagnostic.elf"
    scratch.write_text(widened)
    link[script_index] = scratch
    link[link.index("-o") + 1] = elf
    native_tool(tool, *link)
    symbols = native_tool("common/bin/objdump", "-t", elf)

    def sym(name):
        values = re.findall(r"^([0-9a-fA-F]+) .*\s" + name + r"$", symbols, re.M)
        if len(values) != 1:
            raise SystemExit(f"Expected one linker symbol {name}, got {len(values)}")
        return int(values[0], 16)

    used = sym("_data_load") - build.APP_XIP + sym("_data_end") - sym("_data_start")
    image = (used + 3) // 4 * 4 + 4
    pool = sym("_pool_end") - sym("_pool_start")
    if sym("_pool_start") != 0x01C20000 or not 0 <= pool <= 0x54000:
        raise SystemExit("Diagnostic pool exceeds the unchanged stock region")
    print(json.dumps({
        "model": build.MODEL,
        "diagnostic_elf_only": True,
        "stock_xip_window": build.APP_CODE_MAX,
        "would_be_application_bytes": image,
        "stock_xip_free": build.APP_CODE_MAX - image,
        "xip_floor": 1024,
        "pool_used": pool,
        "pool_margin_after_reserve": 0x54000 - pool - 8192,
        "pool_margin_floor": 512,
        "normal_ram_used": sym("_bss_end") - 0x01C08000,
    }, sort_keys=True), flush=True)
    raise SystemExit(0)


if __name__ == "__main__":
    build.tc = measure
    build.main()
