import ctypes as C,json, resource,sys, subprocess,signal
from pathlib import Path
if len(sys.argv)>2:
 resource.setrlimit(resource.RLIMIT_CORE,(0,0))
 o=Path(sys.argv[1]); k=sys.argv[2]
 cfg=Path('sj6-cross-preset/sub-range.private-output/2-dry.private.txt')
 l=cfg.read_text().splitlines();n=int(l[1]);mut={'cutoff':{7:'.8',180:'.8'},'pitch':{57:'.5'}}[k]
 for i in range(2,2+n):
    x=float(l[i].split()[0])
    if x in mut:l[i]=f'{int(x)} {mut[x]}'
 p=o/f'reject-{k}.txt';p.write_text('\n'.join(l)+'\n')
 src=Path('sj6-current-faders/probe.py'); ns={};exec(compile(src.read_text().split('cases={}')[0],str(src),'exec'),ns);e=ns['e']
 co=(C.c_float*5)(*[e.rf(a) for a in (0x10063739c,0x1006373a0,0x1006373a4,0x1006373a8,0x100637400)])
 ph=json.load(open('sj6-raw-main-ota/physical.json'));pa=(C.c_float*4)(*ph['physical_controls'][0])
 dll=C.CDLL(str(o/'combined.so'))
 dll.select_ota.argtypes=[C.c_uint,C.c_void_p,C.c_void_p];dll.select_ota(3,pa,co)
 dll.run_render.argtypes=[C.c_int,C.POINTER(C.c_char_p)]
 a=(C.c_char_p*3)(b'probe',str(p).encode(),str(o/f'reject-{k}.f32').encode());assert dll.run_render(3,a)==0
 sys.exit(0)
o=Path(sys.argv[1]);res={}
for k in ('cutoff','pitch'):
 p=subprocess.run([sys.executable,__file__,str(o),k],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
 assert p.returncode==-signal.SIGABRT,(p.returncode,p.stderr.decode())
 f=o/f'reject-{k}.f32';n=f.stat().st_size if f.exists() else 0
 assert n<=16384*8
 res[k]=dict(failed_closed=True,frames_before_abort=n//8,reason=p.stderr.decode().splitlines()[-1:] if p.stderr else [])
print(json.dumps(res,indent=2))
