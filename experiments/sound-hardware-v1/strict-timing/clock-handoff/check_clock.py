"""Exercise the candidate C interrupt and startup functions on a controllable ALNK clock."""
import argparse
from pathlib import Path
import subprocess
import tempfile

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("tree", type=Path, help="Firmware checkout with the candidate patch applied")
    args = parser.parse_args()
    source = (args.tree / "firmware/src/app/main_fm1.c").read_text()
    begin = source.index("static void smk_usb_boot_settle(void)\n{")
    end = source.index("static void audio_init(void)", begin)
    irq = source[begin:end]
    vector = "void fm1_alnk0_boot_irq(void)" in irq
    main_at = source.index("static void fm1_main(void)\n{")
    guard = "    fm1_guard_lock_top();\n"
    begin = source.index(guard, main_at) + len(guard)
    end = source.index("    fm1_delay_ms(30);", begin)
    template = Path(__file__).with_name("clock.c").read_text()
    with tempfile.TemporaryDirectory() as folder:
        src = Path(folder) / "clock.c"
        src.write_text(template.replace("/* FIRMWARE_IRQ */", irq).replace("/* FIRMWARE_HANDOFF */", source[begin:end]))
        exe = Path(folder) / "check"
        subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-O2",
                        f"-DVECTOR={int(vector)}", src, "-o", exe], check=True)
        subprocess.run([exe], check=True)

if __name__ == "__main__":
    main()
