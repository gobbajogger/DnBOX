import sys, struct, ctypes as C, json
from pathlib import Path
import numpy as np
import emu, macho
from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import UC_CPU_ARM64_MAX

def new_uc(*args, **kwargs):
    result = constructor(*args, **kwargs)
    result.ctl_set_cpu_model(UC_CPU_ARM64_MAX)
    return result

constructor = emu.Uc
emu.Uc = new_uc
macho.EXPECTED_SHA = '6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
base = Path(__file__).resolve().parent
lib = C.CDLL(str(base / (sys.argv[1] if len(sys.argv)>1 else 'candidate.so')))
lib.create.argtypes = (C.c_uint,)
lib.create.restype = C.c_void_p
lib.juno6_classic_native_process.argtypes = (C.c_void_p, C.c_void_p, C.c_void_p, C.c_uint)

def run(mode, cue):
    e = emu.Emu(str(base.parent/'sj6-modern-private-recovery/arm64.private'))
    create, init, render, setp = 0x1003bc888, 0x1003bcad0, 0x1003bcf58, 0x1003bccec
    assert e.im.sym_by_name['_createChorusDSP'] == create
    node = e.call(create)
    e.call(init,node,2,doubles=(44100.,))
    ib,ob,inp,out=e.calloc(4096),e.calloc(4096),e.calloc(40),e.calloc(40)
    e.w64(node+0x18,inp);e.w64(node+0x20,out);e.wr(node+0x29,b'\x01')
    rate=(.513,.863,1.726)[mode-1]
    mix=.555 if mode==3 else .505
    for i,x in enumerate((rate,.97*.97*.202,0.,mix,0.)):
        e.call(setp,node,i,1,floats=(x,))
    e.call(setp,node,5,1,floats=(float(mode),))
    compact=lib.create(mode+1)
    if __import__('os').getenv('TRACE'):
        for o in (0x68,0x70,0xa0,0xd0,0x100,0x128):
            print(hex(o),e.rd(node+o,16).hex(),file=sys.stderr)
        def hook(uc, addr, sz, data):
            if len(trace) < 64:
                x=e.rd(node+0xd0,4)
                trace.append((hex(addr), e.s(0),e.d(0),e.s(8),e.s(3),[e.rf(node+0x38+i) for i in (0,4,8,12,16,20,24)],struct.unpack('<f',x)[0]))
        trace=[]
        for addr in (0x1003bd1b0,0x1003bd1d0,0x1003bd1e0,0x1003bd250,0x1003bd25c,0x1003b9520,0x1003b9550,0x1003b9560):
            e.uc.hook_add(UC_HOOK_CODE,hook,begin=addr,end=addr)

    ref=np.zeros_like(cue)
    candidate=np.zeros_like(cue)
    for at in range(0,cue.shape[1],80):
        n=min(80,cue.shape[1]-at)
        e.w64(node+0x30,at)
        for p,b in ((inp,ib),(out,ob)):
            e.wr(p,struct.pack('<IIIIQIIQ',2,0,1,n*4,b,1,n*4,b+2048))
        for ch in range(2):e.wr(ib+2048*ch,cue[ch,at:at+n].tobytes())
        e.call(render,node,n,0)
        for ch in range(2):ref[ch,at:at+n]=np.frombuffer(e.rd(ob+2048*ch,n*4),'<f4')
        c=np.ascontiguousarray(cue[:,at:at+n])
        lib.juno6_classic_native_process(compact,c[0].ctypes.data,c[1].ctypes.data,n)
        candidate[:,at:at+n]=c
    assert not e.unknown,e.unknown
    if __import__('os').getenv('TRACE'): print(trace[:14],file=sys.stderr)
    # Compare the naked source kernel before the app's external booster.
    candidate/=np.float32(1.707)
    return ref,candidate

def db(a,b):
    return float(20*np.log10(max(np.linalg.norm(a),1e-20)/max(np.linalg.norm(b),1e-20)))

out=[]
for mode in range(1,4):
    pulse=np.zeros((2,6144),dtype='<f4')
    pulse[:,[0,1777,3554]]=[[.5,-.4,.31],[-.5,.4,-.31]]
    ref,got=run(mode,pulse)
    a={"mode":mode,"full_residual_dBr":db(ref-got,ref),"events":[]}
    for ch in range(2):
        for at in (0,1777,3554):
            window=slice(at+100,at+1300)
            r,g=ref[ch,window],got[ch,window]
            a['events'].append(dict(ch=ch,start=at,ref_lag=int(np.argmax(abs(r)))+100,got_lag=int(np.argmax(abs(g)))+100,level_error_dB=db(g,r),error_dBr=db(g-r,r)))
    out.append(a)
print(json.dumps(out,indent=2))
