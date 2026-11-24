from pathlib import Path
from PIL import Image
from hashlib import sha256
import sys
root=Path(sys.argv[1])
cases={'catalog-off':None,'catalog-full':None,'ui_sound_reachable_capture':14,
       'ui_arp_endpoint_capture':32,'ui_synth_focus_capture':98,'ui_lfo_wave_glyph_capture':26}
keep=('STATE ','PAGE ','EXTRA ','MENU ','PHYSICAL ','PASS ','A-','B-','A S','B S')
for test,expected in cases.items():
 a,b=(root/x for x in ('pre','post'))
 def norm(path):
   lines=path.read_text().splitlines()
   assert not any(line.startswith('FAIL') for line in lines), path
   return [l for l in lines if l.startswith(keep)]
 before,after=(norm(x/f'{test}.log') for x in (a,b))
 assert before and before==after, test
 if test.startswith('catalog'):
   print(f'{test}:',before[-1])
   print('  matches all',sum(l.startswith('PAGE ') for l in after),'page rows,',
         sum(l.startswith('EXTRA ') for l in after),'raw tail refs,',
         sum(l.startswith('PHYSICAL ') for l in after),'physically visited list entries')
   for side,size in [(a,500),(b,444)]:
      assert f'SIZE page materialization={size} ref=4' in (side/f'{test}.log').read_text()
 png=sorted((a/test).glob('*.png')); postpng=sorted((b/test).glob('*.png'))
 assert len(png)==len(postpng) and (expected is None or len(png)==expected),test
 for f,g in zip(png,postpng):
   assert f.name==g.name and Image.open(f).convert('RGB').tobytes()==Image.open(g).convert('RGB').tobytes(), (test,f)
 print('  PASS',test,len(png),'paired frames pixel-exact; same semantic log SHA256',sha256('\n'.join(after).encode()).hexdigest())
