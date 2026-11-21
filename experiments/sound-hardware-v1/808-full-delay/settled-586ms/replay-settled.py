"""Replay the recorded 5,500-block settled 8W8 interpolator comparison."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("checkout", type=Path, help="DnBOX checkout of e3ffb3d8")
args = parser.parse_args()
root = args.checkout.resolve()
here = Path(__file__).resolve().parent
dsp = root / "firmware/src/dsp"
kernel = (here / "settled-fxbus.c.inc").read_text()
regular = kernel[:kernel.index("    /* At this magnitude")] + kernel[kernel.index("    } else for(int i=0;i<frames;i++){"):].replace("    } else for", "    for", 1)
regular = regular.replace("void fxbus_808_delay(", "void ref_delay(", 1)

with tempfile.TemporaryDirectory() as temp:
    directory = Path(temp)
    source = directory / "src/dsp"
    source.mkdir(parents=True)
    generated = directory / "generated"
    generated.mkdir()
    subprocess.run([sys.executable, root / "tools/gen_drum_samples.py", generated / "x0x_drum_samples.h"], cwd=root, check=True)
    shutil.copy(dsp.parent / "x0x_cold.h", source.parent)
    header = (dsp / "fxbus.h").read_text()
    header = header.replace("#define FX_808_DELAY_FRAMES 19200", "#define FX_808_DELAY_FRAMES 26112\n#define FX_808_DELAY_WORDS FX_808_DELAY_FRAMES")
    assert "FX_808_DELAY_WORDS" in header
    (source / "fxbus.h").write_text(header)
    existing = (dsp / "fxbus.c").read_text()
    assert existing.count("void fxbus_808_delay(") == 1
    existing = existing.replace("void fxbus_808_delay(", "void e3_delay(")
    (source / "fxbus.c").write_text(existing + "\n#define D8_QUIET (FX_808_DELAY_FRAMES + 1)\n" + regular + "\n" + kernel)
    for optimization in ("-O0", "-O2"):
        executable = directory / ("check" + optimization[-1])
        subprocess.run(["cc", optimization, "-ffp-contract=off", "-fsanitize=undefined",
                        "-fno-sanitize-recover=undefined", "-ffunction-sections", "-fdata-sections",
                        "-Wl,--gc-sections", "-I" + str(source), "-I" + str(dsp),
                        "-I" + str(generated), here / "settled-exact.c", source / "fxbus.c",
                        dsp / "x0x_param.c", dsp / "drum909.c", "-lm", "-o", executable], check=True)
        subprocess.run([executable], check=True)
