import json,subprocess,sys
from pathlib import Path
here=Path(__file__).resolve().parent
p=here.parent/'s1-fx-parity/reference-build'
d=Path(sys.argv[1]).resolve()
commands=json.loads((p/'factory-build-commands.json').read_text())
c=next(x.copy() for x in commands if '-c' in x and x[x.index('-c')+1].endswith('/preset_audio.cpp'))
for f,v in [('-c',d/'orig.cpp'),('-o',d/'original.o'),('-MF',d/'original.d')]: c[c.index(f)+1]=str(v)
c.insert(1,'-I'+str(here.parent/'s1-fx-parity'))
subprocess.run(c,check=True)
link=commands[-1].copy();link[link.index(str(p/'factory-6.o'))]=str(d/'original.o');link[link.index('-o')+1]=str(d/'original')
subprocess.run(link,check=True)
(d/'ref-build.json').write_text(json.dumps([c,link],indent=2)+'\n')
