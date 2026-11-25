import numpy as np, subprocess, pathlib, json, concurrent.futures
d=pathlib.Path('fit-room-why')
O='s1-fx-parity/known-stock-validation/original'
M='s1-fx-parity/reference-build/factory-inputs/public-morphs.f32'
C='s1-fx-parity/reference-build/commands/preset-commands.bin'
pack='fw/build/smk37-elite/fdata.img'
def jobs(i,oi):
 for mode in ('dry','delay','reverb','wet'):
  for side,opts in [('reference',[O,M,C,str(oi)]),('old',['fit-s1-ab/compact',pack,str(i)]),('new',['fit-organ-unified/compact',pack,str(i)])]:
   f=d/f'{i}-{side}-{mode}.f32'
   yield (f,opts+[str(f),mode])
def run(x):
 f,cmd=x
 subprocess.run(cmd,check=True,stdout=subprocess.DEVNULL)
def rms(a):return np.sqrt((a*a).mean())
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as ex:list(ex.map(run, [job for pair in [(62,62),(113,113),(128,128),(278,289),(435,435)] for job in jobs(*pair)]))
for i in (62,113,128,278,435):
 def read(s,m):return np.fromfile(d/f'{i}-{s}-{m}.f32','<f4').reshape(-1,2)
 dry={s:read(s,'dry') for s in ('reference','old','new')}
 print(i, 'dry byte-identical',np.array_equal(dry['old'],dry['new']))
 for mode in ('delay','reverb','wet'):
  a={s:read(s,mode)-dry[s] for s in dry}
  print(mode, 'unchanged', np.array_equal(a['old'],a['new']))
  for t,w in [(0,264600),(220500,264600)]:
   z={s:rms(v[t:w]) for s,v in a.items()}
   print(t,{s:round(float(20*np.log10(z[s]/z['reference'])),3) for s in ('old','new')} if z['reference'] else z)
