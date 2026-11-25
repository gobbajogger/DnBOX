"""Stage Remote control sources onto an already composed, writable firmware tree.
This does not run builds, fit analysis, host tests, emulators or publication.
"""
from argparse import ArgumentParser
from pathlib import Path
import shutil
import subprocess

p = ArgumentParser(description=__doc__)
p.add_argument("tree", type=Path)
p.add_argument("--sj6", type=Path, required=True, help="owner-specific compose adapter")
p.add_argument("--transport", type=Path, required=True, help="existing verified DNR3 header")
p.add_argument("--protocol-donor", type=Path, required=True,
               help="exact typed trial remote_dnr3.c preimage")
a = p.parse_args()
kit = Path(__file__).resolve().parent
app = a.tree / "firmware/src/app"
panel = app / "ui.c"
ui = panel.read_text()
if "int ui_remote_pref(" not in ui:
    ui += """
#if X0X_REMOTE_DNR3
int ui_remote_pref(const uint8_t *e,int set,int value,const x0x_param_t **dp)
{
    pref_t r;
    memcpy(&r,e,4);
    const x0x_param_t *d = pref_desc(r);
    *dp = d;
    if (!d || (set && pref_read_only(r))) return -1;
    if (set) pref_set_impl(r,value,0);
    return pref_get(r);
}
#endif
"""
if '#include "remote_synth_panel.inc"' in ui:
    raise SystemExit("already staged: use a clean composed working tree")
for name, source in (
    ("remote_synth_panel.inc", kit / "remote_synth_panel.inc"),
    ("remote_s1_panel.inc", kit / "remote_s1_panel.inc"),
    ("remote_sj6_panel.inc", a.sj6),
    ("remote_dnr3.c", a.protocol_donor),
    ("remote_dnr3.h", a.transport),
):
    shutil.copyfile(source, app / name)
panel.write_text(ui + '\n#if X0X_REMOTE_DNR3 && X0X_REMOTE_TYPED_SYNTH\n'
                 '#include "remote_synth_panel.inc"\n#endif\n')

with (kit / "protocol-all-controls.patch").open("rb") as delta:
    subprocess.run(["patch","--batch","--fuzz=0","--no-backup-if-mismatch","-p1"],
                   cwd=a.tree,stdin=delta,check=True)
