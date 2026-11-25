"""Controlled fifth-rate vs frozen compact vs Build9-derived 250ms oracle."""
import sys,tempfile,json,time,statistics
from pathlib import Path
import numpy as np
from scipy.signal import firwin
D=Path(__file__).resolve().parent.parent
sys.path[:0]=[str(D/'sj6-fx-bench'),str(D/'sj6-multirate')]
import bench,verify_stream as vs
SR=44100; N=2048; SIZE=44100; DELAY=11025; fb=.99*64/127
def rms(x):return np.sqrt(np.mean(np.square(x,dtype=np.float64)))
def db(x,y): return float(20*np.log10(max(x,1e-16)/max(y,1e-16)))
def render(so,x,coef,wet):
 o=np.empty_like(x); ptr=so.stream_new()
 try:
  assert so.stream_params(ptr,DELAY,fb,.1 if wet else 1.)
  so.stream_process(ptr,x.shape[1],*x,*o,coef)
  assert np.isfinite(o).all()
  return o
 finally:so.stream_free(ptr)
def col(x,f,a,b):
 sl=x[a:b].astype(float)
 t=np.arange(len(sl));w=np.hanning(len(sl))
 return float(abs(np.dot(sl*w,np.exp(-2j*np.pi*f*t/SR)))/sum(w))
def main():
 results={}; coef=np.rint(firwin(37,4000,fs=SR,window=('kaiser',5))*32768).astype('i2')
 with tempfile.TemporaryDirectory() as tmp:
  so=vs.load(D/'sj6-multirate',tmp,37,5)
  assert so.stream_ring()==3948 and so.stream_state()==4092
  for f in (220,1000,3000,6000):
   t=np.arange(N); x=np.zeros((2,SIZE),np.float32)
   x[0,:N]=.12*np.sin(np.pi*t/N)**2*np.sin(2*np.pi*f*t/SR)
   ys={}
   for nm,fn in [
    ('original',lambda e:bench.audited_delay_reference(x,e,D/'sj6-multirate')),
    ('fitted',lambda e:bench.compact(D/'fit-s1-ab/sj6.so','delay',x,e)),
    ('3bit',lambda e:render(so,x,coef,e))]:
    dry=fn(False)
    assert np.array_equal(dry.view('u4'),x.view('u4'))
    ys[nm]=fn(True)-dry
    assert np.all(ys[nm][1]==0)
   for k in (1,2):
    j=k*DELAY; start=j-32; end=j+N+32
    orig=ys['original'][0,start:end]
    record={}
    for nm in ('fitted','3bit'):
     test=ys[nm][0,start:end]
     record[nm]={'rms_error_db':round(db(rms(test),rms(orig)),3),
        'null_rel_db':round(db(rms(test-orig),rms(orig)),3),
        'intended_frequency_level_error_db':round(db(col(ys[nm][0],f,start,end),col(ys['original'][0],f,start,end)),3)}
     if f==6000:
      record[nm]['2820hz_image_rel_to_original_6000hz_db']=round(db(col(ys[nm][0],2820,start,end),col(ys['original'][0],f,start,end)),3)
    results[f'{f}:{k}']=record
  print(json.dumps({'rates':SR,'delay_samples':DELAY,'feedback':fb,'arena_bytes':4096,'stream_state_native_and_host':so.stream_state(),'stream_ring_bytes':so.stream_ring(),'dry_bypass':'all three bit-exact on all four stimuli, no crosstalk','results':results},indent=2))
if __name__=='__main__':main()
