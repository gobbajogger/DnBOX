#!/usr/bin/env python3
"""Compare compact preset transitions with the pinned source-derived main pass."""
import argparse
import json
import math
import os
from pathlib import Path
import runpy
import struct
import subprocess
import tempfile
from unittest.mock import patch
from uuid import UUID

from compile_bank import CATALOG, compile_bank

HERE = Path(__file__).resolve().parent
REFERENCE = "5d18b182955d107c7f8ca09fb5b0b08a83a6adc9"
PACKAGE = "experiments/superj6-conductor-source-20261115"
PWM_IDS = (78, 79, 210, 211)


def synthetic_factory():
    controls = {row["id"]: row["identifier"] for row in json.loads(CATALOG.read_bytes())["controls"]}
    programs = {}
    for position in range(128):
        wave = (position // 32) / 3.0
        explicit_zero = position & 16
        uid = str(UUID(int=position + 1))
        fields = {0: 0.5, 82: wave, 83: wave, 119: 0.0, 120: 0.0, 121: 1.0, 122: 0.0}
        for bit, (field, value) in enumerate(zip(PWM_IDS, (0.25, 0.75, 1.0, 1.0))):
            if position & (1 << bit):
                fields[field] = 0.0 if explicit_zero else value
        programs[uid] = {"uid": uid, "position": position, "parameterValues": [
            {"identifier": controls[field], "value": value} for field, value in fields.items()
        ]}
    return {str(UUID(int=0)): ({"name": "Synthetic PWM presence"}, programs)}


def expected_words(out, programs):
    for name in ("conductor.py", "control_contract.json"):
        ref = f"{REFERENCE}:{PACKAGE}/{name}"
        (out / name).write_bytes(subprocess.check_output(["git", "show", ref], cwd=HERE))
    model = runpy.run_path(str(out / "conductor.py"))
    rows = []
    capabilities = frozenset(("control_apply", "delay_enable_prepare"))
    for program in programs.values():
        records = tuple(model["Record"](e["identifier"], e["value"]) for e in program["parameterValues"])
        main = model["plan_main_pass"](records, b"", 1, True, 256, capabilities, math.pow)
        if main.status != "OK_MAIN_PASS_ONLY":
            raise RuntimeError(main.status)
        selected = [cmd for cmd in main.commands if cmd.id in PWM_IDS]
        if tuple(cmd.id for cmd in selected) != PWM_IDS:
            raise RuntimeError("PWM main-pass order changed")
        words = [struct.unpack("<I", struct.pack("<f", cmd.value))[0] for cmd in selected]
        rows.append("    {" + ", ".join(f"0x{word:08x}u" for word in words) + "}")
    (out / "pwm_expected.inc").write_text(
        "static const uint32_t expected[][4] = {\n" + ",\n".join(rows) + "\n};\n"
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("compact_tree", type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="sj6-pwm-") as tmp:
        out = Path(tmp)
        factory = synthetic_factory()
        expected_words(out, next(iter(factory.values()))[1])
        # Generate only synthetic test data. The real factory entry still
        # authenticates all twelve original members in load_factory().
        with patch("compile_bank.load_factory", return_value=factory):
            compile_bank(None, out / "bank_data.inc")
        env = dict(os.environ, SMC_UI_OUT=tmp, SMC_UI_TEST_FILE=str(HERE / "pwm_defaults_test.c"),
                   X0X_HOST_DEFS=f"-Itests/host -I{out} -Wl,--wrap=juno6_init")
        subprocess.run(["sh", "tests/host/smc_ui_test.sh"], cwd=args.compact_tree, env=env, check=True)


if __name__ == "__main__":
    main()
