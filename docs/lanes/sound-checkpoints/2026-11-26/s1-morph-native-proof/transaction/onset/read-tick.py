from pathlib import Path
from array import array
import hashlib,json,struct,subprocess,sys
e,r,p=sys.argv[1:]; ram=Path(r).read_bytes(); pcm=Path(p).read_bytes()
syms={}
for line in subprocess.check_output(['readelf','-sW',e],text=True).splitlines():
 c=line.split()
 if len(c)>=8 and c[6]!='UND' and c[2].isdigit():syms[c[7]]=(int(c[1],16)-0x01c00000,int(c[2]))
def data(k,fmt):
 a,n=syms[k]; assert n>=struct.calcsize(fmt) and a>=0
 z=struct.unpack_from(fmt,ram,a); return z[0] if len(z)==1 else z
samples=array('h',pcm)
print(json.dumps({'timeline_injected_909_frame_pre909L_pre909Rev_postMasterL':data('d9_test_timeline','<4I'),
 'rendered_frames':data('d8_frame','<I'),'late':data('audio_xruns','<I'),'frames':len(samples)//2,
 'first_pcm_nonzero_frame':next((i//2 for i,v in enumerate(samples) if v),None),
 'pcm_sha256':hashlib.sha256(pcm).hexdigest()}))
