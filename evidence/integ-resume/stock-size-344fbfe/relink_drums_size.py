#!/usr/bin/env python3
"""Run the stock plan with only the three changed translation units recompiled."""
from pathlib import Path
import sys

# Reuse the diagnostic-only linker from the prior composition receipt.
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "stock-compose-7233317"))
from measure_failed_stock_link import build, measure

plan = build.app_compile_commands


def narrow_plan(flags, original, wobble):
    units, objects = plan(flags, original, wobble)
    changed = {"drum808.c", "drum909.c", "breaks.c"}
    substitutions = {}
    selected = []
    for unit in units:
        if "-c" not in unit or Path(unit[unit.index("-c") + 1]).name not in changed:
            continue
        args = list(unit)
        index = args.index("-o") + 1
        cached = Path(args[index])
        output = cached.with_name("measure-cold-" + cached.name)
        args[index] = output
        substitutions[cached] = output
        selected.append(tuple(args))
    if len(selected) != len(changed):
        raise SystemExit("Expected only the three changed units")
    objects = [substitutions.get(Path(path), path) for path in objects]
    for path in objects:
        if path not in substitutions.values() and not Path(path).is_file():
            raise SystemExit(f"Missing exact cached object: {path}")
    return selected, objects


if __name__ == "__main__":
    build.app_compile_commands = narrow_plan
    build.tc = measure
    build.main()
