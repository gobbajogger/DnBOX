#!/usr/bin/env python3
"""Read-only analysis of the frozen native resource receipt.

This decodes fixed native frames and direct call edges. It does not execute
target code, replace a runtime stack watermark, or measure native cycles.
"""
from pathlib import Path
import hashlib
import json
import re

HERE = Path(__file__).resolve().parent
OUT = HERE / "native"
result = json.loads((OUT / "RESULT.json").read_text())
baseline = json.loads((HERE.parent / "native-linked-ledger.json").read_text())


def functions(disassembly):
    matches = list(re.finditer(r"^([A-Za-z_][A-Za-z_0-9.]*):\s*$", disassembly, re.M))
    return {m.group(1): disassembly[m.end():matches[i + 1].start() if i + 1 < len(matches) else None]
            for i, m in enumerate(matches)}


def register_count(registers):
    count = 0
    for item in registers.split(","):
        item = item.strip()
        match = re.fullmatch(r"r(\d+)-r(\d+)", item)
        if match:
            count += abs(int(match.group(1)) - int(match.group(2))) + 1
        elif re.fullmatch(r"(?:r\d+|rets|pc)", item):
            count += 1
        else:
            raise ValueError("Unrecognized saved-register token: " + item)
    return count


def frame_info(body, address_names=None):
    pushes = re.findall(r"\[--sp\]\s*=\s*\{([^}]+)\}", body)
    subtracts = [int(n) for n in re.findall(r"\bsp\s*\+=\s*-(\d+)\b", body)]
    unknown = re.findall(r"(?<![\w])sp\s*=\s*[^\n]+", body)
    # Candidate functions have a single fixed entry save/allocation. Refuse
    # to label branch-dependent or dynamically allocated stacks as proven.
    if len(pushes) > 1 or len(subtracts) > 1 or unknown:
        return {"recognized": False, "pushes": pushes, "subtractions": subtracts,
                "unknown_stack_writes": unknown}
    saved = 4 * sum(map(register_count, pushes))
    local = sum(subtracts)
    callees = set()
    unknown_calls = []
    for line in body.splitlines():
        if not re.search(r"\bcall\b", line):
            continue
        call = re.search(r"^\s*([0-9a-f]+):.*?\bcall\s+(-?\d+)(?:\s+<([^ :>]+)\s*:)?", line)
        if not call:
            unknown_calls.append(line)
            continue
        name = call.group(3)
        if not name and address_names is not None:
            # Native PC-relative direct calls are four bytes long. A few
            # conditional calls lack objdump's textual target annotation.
            name = address_names.get(int(call.group(1), 16) + 4 + int(call.group(2)))
        if name:
            callees.add(name)
        else:
            unknown_calls.append(line)
    if unknown_calls:
        return {"recognized": False, "unknown_calls": unknown_calls,
                "saved_register_bytes": saved, "local_and_outgoing_bytes": local}
    # A tail goto is conservatively counted while retaining the caller frame.
    # Candidate tail callees have zero frame in this compile; this cannot
    # understate the direct-call upper bound.
    tail_callees = sorted(set(re.findall(r"\bgoto\s+[-\d]+\s+<([^ :>]+)\s*:", body)))
    return {"recognized": True, "saved_register_bytes": saved,
            "local_and_outgoing_bytes": local, "fixed_frame_bytes": saved + local,
            "callees": sorted(callees), "potential_tail_callees": tail_callees}


summary = {"provenance": result["provenance"], "native_layout": result["native_layout"],
           "method": "ELF32-pi32v2 linked disassembly fixed frames and direct calls, four bytes per saved register; no target execution",
           "limitations": ["Caller and interrupt frames excluded", "No hardware stack watermark", "No native timing or cycle conversion", "Resource composition has no candidate runtime instances or engine routing"],
           "builds": []}
soft_re = re.compile(r"\s(__(?:add|sub|mul|div|neg|cmp|eq|ne|lt|le|gt|ge|un|extendsf|truncdf|fixdf|fixunsdf|floatsidf|floatunsidf|floatdidf)\w*df\w*)$", re.M)

for link in result["full_additive_links"]:
    model, opt = link["model"], link["optimization"]
    label = "additive-" + model + "-" + opt
    funcs = functions((OUT / (label + "-disassembly.log")).read_text())
    address_names = {s["address"]: s["name"] for s in link["symbols"]
                     if s["section"] == ".text" and s["bytes"]}
    object_funcs = {s["name"] for s in result["objects_without_lto"][opt]["symbols"]
                    if s["section"].startswith(".text") and s["bytes"]}
    infos = {}
    missing = set()

    def collect(name):
        if name in infos:
            return
        if name not in funcs:
            missing.add(name)
            return
        info = frame_info(funcs[name], address_names)
        infos[name] = info
        if info["recognized"]:
            # Internal basic-block targets are not interprocedural edges.
            edges = [n for n in info["callees"] if n != name]
            for n in info["potential_tail_callees"]:
                if n != name and n in object_funcs:
                    edges.append(n)
            info["edges"] = sorted(set(edges))
            for child in info["edges"]:
                collect(child)

    for name in link["public_link_roots"]:
        collect(name)

    memo = {}

    def depth(name, parents=()):
        if name in memo:
            return memo[name]
        if name in parents:
            raise ValueError("Recursion detected: " + " -> ".join((*parents, name)))
        if name not in infos or not infos[name]["recognized"]:
            return {"complete": False, "path": [name]}
        children = [depth(c, (*parents, name)) for c in infos[name]["edges"]]
        if any(not c["complete"] for c in children):
            return {"complete": False, "path": [name]}
        child = max(children, key=lambda c: c["bytes"], default={"bytes": 0, "path": []})
        answer = {"complete": True, "bytes": infos[name]["fixed_frame_bytes"] + child["bytes"],
                  "path": [name, *child["path"]]}
        memo[name] = answer
        return answer

    paths = {name: depth(name) for name in link["public_link_roots"]}
    sizes = result["objects_without_lto"][opt]["sections"]
    base_sections = baseline[model]["sections"]
    baseline_bytes = sum(base_sections[n]["size"] for n in [".text", ".ram_text", ".c1_text", ".data"]) + 4
    syms = (OUT / (label + "-symbols.log")).read_text()
    row = {"model": model, "optimization": opt, "public_api_count": len(link["public_link_roots"]),
           "object_text_bytes": sum(v["bytes"] for k, v in sizes.items() if k.startswith(".text")),
           "object_rodata_bytes": sum(v["bytes"] for k, v in sizes.items() if k.startswith(".rodata")),
           "object_mutable_static_bytes": sum(v["bytes"] for k, v in sizes.items() if k.startswith((".data", ".bss"))),
           "object_undefined": result["objects_without_lto"][opt]["undefined"],
           "linked_undefined": link["undefined"], "soft_double_symbols": sorted(set(soft_re.findall(syms))),
           "integer_64bit_helper_symbols": sorted(set(re.findall(r"\s(__\w*di[23])$", syms, re.M))),
           "baseline_image_bytes": baseline_bytes,
           "composition_image_bytes": link["load_image_bytes_including_end_marker"],
           "incremental_image_bytes": link["load_image_bytes_including_end_marker"] - baseline_bytes,
           "physical_free_bytes_including_end_marker": link["physical_xip_free"],
           "shortfall_from_1024_free_gate": 1024 - link["physical_xip_free"],
           "physical_link_returncode": link["physical_link_returncode"], "xip_gate": link["xip_gate"],
           "unchanged_ram_sections": {n: link["sections"][n]["bytes"] for n in [".data", ".bss", ".pool", ".xpool", ".c1stack", ".noinit"]
                                      if link["sections"][n]["bytes"] == base_sections[n]["size"]},
           "stack_analysis": {"frames": infos, "api_paths": paths, "missing_functions": sorted(missing)}}
    summary["builds"].append(row)

current = {}
for file in result["provenance"]["files"]:
    path = Path(file["path"])
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    current[str(path)] = {"sha256": digest, "matches_snapshot": digest == file["sha256"]}
summary["source_recheck"] = current
(OUT / "ANALYSIS.json").write_text(json.dumps(summary, indent=2))
for b in summary["builds"]:
    print(json.dumps({k: v for k, v in b.items() if k not in ["stack_analysis", "object_undefined", "unchanged_ram_sections"]}))
    print("render_stack", b["stack_analysis"]["api_paths"]["dnbx_render"])
    print("note_on_stack", b["stack_analysis"]["api_paths"]["dnbx_note_on"])
    print("missing_stack_functions", b["stack_analysis"]["missing_functions"])
