import ctypes, runpy, sys, math, json, struct, subprocess
from pathlib import Path
from tempfile import TemporaryDirectory
d=Path(__file__).resolve().parent
a=runpy.run_path(str(d/'lazy_anchor.py'))
x=Path('recovered-sound/sj6-sub-handoff/shared-clock/native_matrix.py').read_text().split('actual=[]')[0]
ns={}
exec(compile(x,'source setup','exec'),ns)
ff=ns['fn'].replace(' def prep(*_):',' pre=[]\n def prep(*_):')
ff=ff.replace('  if relative>=33075-16:',"""  pre.append((relative,e.r32(v+0x340),struct.unpack('<d',e.rd(v+0x308,8))[0]))
  if relative>=33075-16:""")
ff=ff.replace(" if variant=='poly':d.set_param(41,0,True)"," if variant=='duration':d.set_param(15,1.25,True)")
ff=ff.replace("return dict(who=who","return dict(pre=pre,who=who")
exec(compile('def alt('+ff,'source standalone witness','exec'),ns)
class S(ctypes.Structure):
    _fields_=[('y',ctypes.c_double),('mul',ctypes.c_double),('add',ctypes.c_double),('stage',ctypes.c_ubyte)]
with TemporaryDirectory() as t:
    obj=Path(t)/'shadow.so'
    subprocess.run(['cc','-fPIC','-shared','-O2','-ffp-contract=off',str(d/'prep_shadow_int.c'),str(d/'exact_fused.c'),'-lm','-o',str(obj)],check=True)
    l=ctypes.CDLL(str(obj))
    l.shadow_tick.argtypes=[ctypes.POINTER(S),ctypes.c_double]
    l.shadow_entry.argtypes=[ctypes.POINTER(S),ctypes.c_int]+[ctypes.c_double]*3
    cases=[]
    for name,lead,variant in [('held',0,'default'),('midrelease',11025,'default'),('released',22050,'default'),('staggered',11025,'staggered'),('duration',11025,'duration')]:
        res=ns['alt']('sampler1',lead,variant)
        p=dict(ns['cfg']['sampler_control_writes']['sampler1'])
        if variant=='duration':p[15]=1.25
        f32=a['f32']
        r=f32(44100/16)
        bendA=float.fromhex('0x1.c8f87724b5c1dp-3')
        bendD=float.fromhex('0x1.d037e1b1c8ff7p-8')
        s=S()
        def enter(kind,x,y,bend,ticks):
            m=math.exp(-math.log((bend+abs(y-x))/bend)/ticks)
            b=(y+(bend if y>x else -bend))*(1-m)
            l.shadow_entry(ctypes.byref(s),kind,x,m,b)
        enter(1,0.,1.,bendA,int(f32(p[10]*r)))
        checked=0
        for tick,stage,y in res['pre']:
            if lead and tick==33075-lead:enter(4,s.y,0.,bendD,int(f32(p[15]*r)))
            if tick==33075:
                assert s.y==y,(name,tick,s.y.hex(),y.hex())
                break
            assert s.y==y and {1:2,2:3,3:4,4:7}[s.stage]==stage,(name,tick,s.y.hex(),y.hex())
            if l.shadow_tick(ctypes.byref(s),p[13])==3 and s.stage==2:
                enter(3,1.,p[13],bendD,int(f32(p[12]*r)))
            checked+=1
        cases.append(dict(case=name,compared_prep_words=checked,first_anchor=s.y.hex(),commit=res['commit'][0][0]))
    print(json.dumps(dict(source_sha=ns['pin'](ns['BIN']),host_size=l.shadow_size(),cases=cases),indent=2))
