"""Raw authenticated same-graph effect-isolation comparisons (no level/time fitting)."""
from pathlib import Path
import os,sys,json,subprocess,concurrent.futures,hashlib
import numpy as np
D=Path(__file__).resolve().parent; W=D.parent
sys.path.insert(0,str(W/'s1-controlled-ab'))
from measure import margin,floats
b=Path(sys.argv[1]).resolve();out=Path(sys.argv[2]).resolve();out.mkdir(exist_ok=True,mode=0o700)
r=W/'s1-fx-parity/reference-build'
m=r/'factory-inputs/public-morphs.f32';cmd=r/'commands/preset-commands.bin'
meta=json.loads((r/'commands/ORIGINAL-PRESET-COMMANDS.json').read_text())['cases']
rows=[int(x) for x in sys.argv[3:]] or [1,62,113,128,154,188,191,289,435]
def one(n):
 arms={};waves={}
 for mode in ('original',*('shape'+x for x in os.getenv('S1_SHAPES','0,16,48').split(','))):
  f=out/f'{n}-{mode}.pcm';d=str(b/('reference' if mode=='original' else 'bounded'))
  env={x:y for x,y in os.environ.items() if not x.startswith(('S1_RESERVE','S1_DELAY_SHAPE','AB_','ARP_'))}
  if mode.startswith('shape'):env['S1_DELAY_SHAPE']=mode[5:]
  if mode.startswith('shape8192-'):
   env['S1_DELAY_SHAPE']='8192';env['S1_DIAGNOSTIC_HZ']=mode.split('-')[1]
  x=subprocess.run([d,str(m),str(cmd),str(n),str(f),'wet'],env=env,check=True,capture_output=True,text=True)
  waves[mode]=floats(f);arms[mode]={'hash':hashlib.sha256(f.read_bytes()).hexdigest(),
    'receipt':x.stdout.strip().splitlines()+[s for s in x.stderr.splitlines() if s.startswith('scope portable')]}
 for mode in arms:arms[mode]['metrics']=margin(waves['original'],waves[mode])
 return {'stock_row':n,'name':meta[n]['name'],'arms':arms}
with concurrent.futures.ThreadPoolExecutor(4) as e: a=list(e.map(one,rows))
print(json.dumps(dict(scope='Authentic original-source graph on both sides; candidate changes only fed delay engine, not restored compact voices or device',cases=a),indent=2))
