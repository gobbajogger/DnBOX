"""Generate the host-test C table from the observed oracle words."""
from pathlib import Path
import json
import sys
rows = json.loads(Path(sys.argv[1]).read_text())["binary32_input_and_observed_binary32_gain"]
Path(sys.argv[2]).write_text("static const uint32_t observed[][2] = {\n" +
    "".join(f"    {{0x{raw:08x}u, 0x{gain:08x}u}},\n" for raw,gain in rows) + "};\n")
