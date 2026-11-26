import sys,os,importlib.util
from pathlib import Path
r=Path(os.environ['W'])
sys.path.insert(0,str(r/'restored-harness-inspect/helpers'))
import guarded_replay as g
spec=importlib.util.spec_from_file_location("nor",r/'fw/tools/fm1_rescue.py')
nor=importlib.util.module_from_spec(spec);spec.loader.exec_module(nor)
package=r/'out/DnBOX-0.0.27-SMK37-elite-73749f55-test.fwsc'
raw=package.read_bytes(); assert g.sha(raw)==g.STOCK['elite']
base=nor.package_flash_image(raw)
for x in ('base','new'):
 exe=Path(x+'.bin').read_bytes()
 assert 0<len(exe)<65536 and len(exe)%4==0
 image=bytes(g.loader(nor,base,exe+b'\0\xff\xff\xff'))
 assert image[:0x4000]==base[:0x4000] and image[0x4000+0x120+g.MAX:]==base[0x4000+0x120+g.MAX:]
 Path(f'{x}-nor-pristine').write_bytes(image+b'\xff'*(0x100000-len(image)))
 print(x,g.sha(exe),g.sha(image))
