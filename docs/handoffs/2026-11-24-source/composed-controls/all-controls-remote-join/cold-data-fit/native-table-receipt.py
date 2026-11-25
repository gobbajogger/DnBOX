"""Hash exact native read-only bytes independently of their lab placement."""
from hashlib import sha256
from pathlib import Path
import json, re, subprocess, sys

root, elf_tools, receipt = (Path(x).resolve() for x in sys.argv[1:])
wanted = [
    ("slot3","original_miami_a44100","experiments/original_synth/motion_tables_private.h",516),
    ("slot3","original_miami_b44100","experiments/original_synth/motion_tables_private.h",516),
    ("slot3","original_pitch44100","experiments/original_synth/motion_tables_private.h",512),
    ("slot3","original_drive_gain_table","experiments/original_synth/chicago_drive_private.h",256),
    ("fittab","sin_q","firmware/src/dsp/s1c/s1u.c",260),
    ("fittab","original_sound_upper_value","firmware/src/app/original_sound.c",256),
]
scratch = root / "read-only-table-receipt"
scratch.mkdir(exist_ok=True)
decls = []
for _, symbol, filename, _ in wanted:
    raw=(root/filename).read_text()
    candidates = re.findall(r"(?m)^static const [^\n]*?\b"+symbol+r"\[\d*\]\s*=\s*\{[^{}]+\};",raw)
    if len(candidates) != 1:
        raise ValueError((symbol,len(candidates)))
    decls.append(candidates[0].replace("static const", "__attribute__((used)) static const", 1))
source=scratch/"tables.c"; source.write_text("#include <stdint.h>\n"+"\n".join(decls)+"\n")
obj=scratch/"tables.o"
subprocess.run([str(elf_tools/"pi32v2/bin/clang"),"-target","pi32v2","-fdata-sections",
                "-Oz","-c",str(source),"-o",str(obj)],check=True)
wanted += [("fittab","sj6_midi_velocity",None,384)]
out=[]
for zone,symbol,filename,n in wanted:
    o=obj if filename else root/"build/smk37-elite/original/original_runtime.o"
    dest=scratch/"hash.bin"
    subprocess.run([str(elf_tools/"common/bin/objcopy"),"-O","binary","-j",".rodata."+symbol,
                    str(o),str(dest)],check=True)
    raw=dest.read_bytes()
    assert len(raw)==n,(symbol,len(raw),n)
    out.append((zone,".rodata."+symbol,n,sha256(raw).hexdigest()))
receipt.write_text(json.dumps(out,indent=2)+"\n")
print(json.dumps(out))
