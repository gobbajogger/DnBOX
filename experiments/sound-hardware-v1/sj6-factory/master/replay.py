"""Replay the actual AU control oracle, then build the DnBOX owner and PCM tests."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

here = Path(__file__).resolve().parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--compact", required=True, type=Path)
parser.add_argument("--factory", required=True, type=Path)
parser.add_argument("--bench", required=True, type=Path)
parser.add_argument("--banks", required=True, type=Path)
parser.add_argument("--out", required=True, type=Path)
args = parser.parse_args()
compact, factory = args.compact.resolve(), args.factory.resolve()
out = args.out.resolve()
out.mkdir(parents=True, exist_ok=False)

def run(*cmd, cwd=None, env=None):
    subprocess.run([str(x) for x in cmd], cwd=cwd, env=env, check=True)

run(sys.executable, factory/"verify_bank.py", args.banks, out/"bank")
run(sys.executable, here/"master_oracle.py", args.bench.resolve(), args.banks, out/"master.json")
expected = json.loads((here/"summary.json").read_text())["observations_sha256"]
if hashlib.sha256((out/"master.json").read_bytes()).hexdigest() != expected:
    raise RuntimeError("AU control observations differ from the captured oracle")
run(sys.executable, here/"make_table.py", out/"master.json", out/"master_oracle.inc")
for fixture in (factory/"compact_application.c", here/"master_application.c"):
    env = os.environ.copy()
    env.update(SMC_UI_TEST_FILE=str(fixture), SMC_UI_OUT=str(out/fixture.stem),
               X0X_HOST_DEFS=f"-Itests/host -I{factory} -I{out} -I{out/'bank'} -Wl,--wrap=juno6_init")
    run("sh", "tests/host/smc_ui_test.sh", cwd=compact, env=env)
run(os.environ.get("CC", "cc"), "-std=c99", "-O2", "-ffp-contract=off", "-ffunction-sections",
    "-Wl,--gc-sections", "-DX0X_HOST", "-Ifirmware/src", here/"master_transfer.c", "-lm",
    "-o", out/"master-transfer", cwd=compact)
run(out/"master-transfer")
