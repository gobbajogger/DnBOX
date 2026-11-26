import subprocess,hashlib,pathlib,concurrent.futures,numpy as np
p=pathlib.Path(__file__).resolve().parent
w=pathlib.Path('/workspace/scratch/3a7051e761a0')
base=w/'recovered-sound/s1-room-parity/build5/room-delay-master'
pack=w/'recovered-sound/s1-bank-replay/original-private/pack.private'
def run(i):
 a=[str(pack),str(i),'/dev/stdout','wet']
 by=[subprocess.check_output([str(b),*a])[:2116800] for b in (base,p/'compact')]
 assert all(len(x)==2116800 for x in by)
 y=np.frombuffer(by[0],'<f4').reshape(-1,2)
 assert by[0]==by[1]
 hz=np.fft.rfftfreq(44100,1/44100); Y=np.fft.rfft(y[-44100:]*np.hanning(44100)[:,None],axis=0)
 b=np.sqrt(np.mean(abs(Y[(hz>=5000)&(hz<16000)])**2))
 print(i,'whole byte-exact, tail byte-exact, 5–16k source RMS',round(b,8),'sha',hashlib.sha256(by[0]).hexdigest(),flush=True)
with concurrent.futures.ThreadPoolExecutor(4) as pool:
 list(pool.map(run,[1,62,113,128,151,154,191,275,289,366,802,903]))
