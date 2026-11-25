"""Replace only the existing, isolated host render driver with ours."""
import json,subprocess,sys
from pathlib import Path
p=Path(__file__).resolve().parent
build,out=map(lambda a:Path(a).resolve(),sys.argv[1:])
assert not out.exists() and out.parent.is_dir()
cmds=json.loads((build/'factory-build-commands.json').read_text())
c=next(x.copy() for x in cmds if '-c' in x and x[x.index('-c')+1].endswith('/preset_audio.cpp'))
for f,v in [('-c',p/'original.cpp'),('-o',out.with_suffix('.o')),('-MF',out.with_suffix('.d'))]:
    c[c.index(f)+1]=str(v)
subprocess.run(c,check=True)
link=cmds[-1].copy()
link[link.index(str(build/'factory-6.o'))]=str(out.with_suffix('.o'))
link[link.index('-o')+1]=str(out)
subprocess.run(link,check=True)
(out.parent/'reference-driver-commands.json').write_text(json.dumps([c,link],indent=2)+'\n')
