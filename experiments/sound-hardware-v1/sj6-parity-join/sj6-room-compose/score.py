from pathlib import Path
base=Path(__file__).resolve().with_name("fixture-parent.py")
exec(compile(base.read_text().split('rows=[]')[0],str(base),'exec'))
f=np.float32;t=np.arange(n);sec=t/44100
# Signed independent held dry versus wet input, post-HP band-rich asymmetric
# chord and overlapping transient; not represented as an actual app output bus.
e=np.minimum(sec/.0025,1)*np.minimum(np.maximum(.41-sec,0)/.026,1)
s=np.minimum(np.maximum(sec-.192,0)/.009,1)*np.minimum(np.maximum(.443-sec,0)/.017,1)
wet=np.stack([e*(.134*np.sin(2*np.pi*440*sec)+.052*np.sin(2*np.pi*978*sec)+.017*np.sin(2*np.pi*(950*sec+15555*sec**2)))+.055*s*np.sin(2*np.pi*3780*sec),
 e*(.114*np.sin(2*np.pi*521*sec+.4)+.067*np.sin(2*np.pi*1301*sec)+.02*np.sin(2*np.pi*(1100*sec+18990*sec**2)))+.029*s*np.sin(2*np.pi*5550*sec)]).astype('f4')
dry=np.stack([.83*wet[0]+.054*np.sin(2*np.pi*172*sec),.72*wet[1]-.077*np.sin(2*np.pi*212*sec)]).astype('f4')
blocks=[(0,7213,1,.52,.151),(7213,12941,0,.52,.151),(12941,16327,1,.38,.404),(16327,n,0,.38,.404)]
gated=np.zeros_like(wet);return_scale=np.zeros(n,'f4')
master=f(.83);fb=f(.92)
for a,b,on,mix,gain in blocks:
 return_scale[a:b]=f(f(f(.05)+f(f(.90)*f(mix)))*f(gain))
 if on:gated[:,a:b]=wet[:,a:b]
ref,_=original(gated,20000,float(fb))
combined=((dry+ref*return_scale)*master).astype('f4')
hb=np.fft.rfftfreq(n,1/44100)>8000
P=C.POINTER(C.c_float)
class Block(C.Structure):
 _fields_=[('l',P),('r',P),('n',C.c_uint),('open',C.c_uint),('type',C.c_uint),('lim',C.c_uint),('rage',C.c_uint),('mix',C.c_float),('tier',C.c_float)]
def test(lib,pattern,forbid=False):
 create=lib.compose_room_create;create.restype=C.c_void_p
 destroy=lib.compose_room_destroy;destroy.argtypes=[C.c_void_p]
 setfb=lib.compose_room_feedback;setfb.argtypes=[C.c_void_p,C.c_float]
 bypass=lib.compose_probe_disabled;bypass.argtypes=[P,P,C.c_uint]
 render=lib.compose_probe;render.argtypes=[C.c_void_p,C.POINTER(Block),P,P,C.c_uint,C.c_float]
 own=create();assert own and setfb(own,fb) and not setfb(own,1)
 y=dry.copy()
 def ptr(foo,i=0):return foo[i:].ctypes.data_as(P)
 if forbid:
  tmp=dry.copy();bp=Block(ptr(wet[0]),ptr(wet[1]),19,1,2,1,1,.52,.151)
  for slot,value in [('type',1),('lim',0),('rage',0),('n',18),('tier',.5),('l',ptr(tmp[0])),('open',2)]:
   old=getattr(bp,slot);setattr(bp,slot,value)
   assert render(own,C.byref(bp),ptr(tmp[0]),ptr(tmp[1]),19,master)==0 and digest(tmp)==digest(dry),slot
   setattr(bp,slot,old)
  assert bypass(ptr(tmp[0]),ptr(tmp[1]),19)==1 and digest(tmp)==digest(dry)
 for a,b,on,mix,gain in blocks:
  k=0
  while a<b:
   take=min(pattern[k%len(pattern)],b-a);k+=1
   b0=Block(ptr(wet[0],a) if on else None,ptr(wet[1],a) if on else None,take,on,2,1,1,mix,gain)
   assert render(own,C.byref(b0),ptr(y[0],a),ptr(y[1],a),take,master)==1
   a+=take
 destroy(own)
 return y
for m in (12,14):
 lib=C.CDLL(str(D/f'room-compose-{m}.so'))
 y=test(lib,[17,63,251],True)
 z=test(lib,[1,256,99])
 assert digest(y)==digest(z)
 yy=y.astype('f8');rr=combined.astype('f8');dry_only=(dry*master).astype('f8')
 err=yy-rr;wetcombined=rr-dry_only
 def hf(v):return np.fft.rfft(v,axis=1)[:,hb]
 rows=[]
 for a,b,on,mix,gain in blocks:
  w=wetcombined[:,a:b]
  assert np.count_nonzero(w)>0
  rows.append(dict(start=a,end=b,open=on,wet_delta_level=dbr((yy-dry_only)[:,a:b],w),wet_relative_error=dbr(err[:,a:b],w),wet_ref_level=float(np.sqrt(np.mean(w*w)))))
 print(json.dumps(dict(bits=m+6,dry_fixture=digest(dry),wet_fixture=digest(wet),gated_source=digest(gated),signed_orig_leaf=digest(ref),signed_reference_sum=digest(combined),signed_out=digest(y),residual_relative_wet=dbr(err,wetcombined),HF_residual_relative_wet=dbr(hf(err),hf(wetcombined)),residual_relative_full=dbr(err,rr),exact_callback_partition=True,reject_negative_no_wr_no_advance=True,rows=rows)),flush=True)
