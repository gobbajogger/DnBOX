"""Reached actual current Swift AMP release sender and AKSampler phys15."""
from pathlib import Path
import struct
def observe(prefix,raw):
 from swiftrt import SwiftWorld
 from unicorn import UC_HOOK_CODE
 from unicorn.arm64_const import UC_ARM64_REG_PC,UC_ARM64_REG_LR
 e=prefix['Emu'](str(prefix['BIN']))
 w=SwiftWorld(e); e.deferred=[]
 SamplerDSP=prefix['SamplerDSP']
 f32=lambda v:struct.unpack('<f',struct.pack('<f',v))[0]
 cond=e.calloc(0x700);inst=e.calloc(0x700);core=e.calloc(0x700)
 e.w64(0x10082ea30,0x100);e.w64(cond+0x100,inst);e.w64(inst+0x20,core)
 e.w64(0x1008145a0,0x300);e.w64(0x100814678,0x320);e.w64(0x10080cb30,0x380)
 e.w64(0x100814670,0x328);e.w64(0x10080cb28,0x388)
 for i,off in enumerate((0x10082f4d8,0x10082f4e0,0x10082f4e8)):e.w64(off,0x520+8*i)
 seq=e.calloc(0x200);e.w64(inst+0xb0,seq);e.wd(seq+0x38,0.)
 calls=[];dsp={}
 for i,(name,off) in enumerate((('Sub',0x1007e3b98),('OSC1',0x1007e3ba8),('Other',0x1007e3ba0),('OSC2',0x1007e3bb0),('Noise',0x1007e3bb8))):
  o=w.new_obj('_TtC8AudioKit9AKSampler',size=0x500);o.native_imps={'setReleaseDuration:':0x100558078,'setReleaseHoldDuration:':0x100557f1c}
  a=w.new_obj('_TtC8AudioKit18AKSamplerAudioUnit',size=0x500)
  pos=0x400+i*0x18;e.w64(off,pos);e.w64(core+pos,o.addr)
  e.w64(o.addr+0x300,a.addr);e.wd(o.addr+0x320,float('nan'));e.wd(o.addr+0x328,float('nan'));dsp[a.addr]=(name,SamplerDSP(e,44100))
 def endpoint(w,o,e):
  name,d=dsp[o.addr];physical=e.s(0);slot=e.x(2)
  assert slot in (14,15)
  calls.append((name,slot,physical))
  e.deferred.append(lambda:d.set_param(slot,physical,True));return True
 w.sel_handlers['setParameterWithAddress:value:']=endpoint
 # UI and FILTER-link notification arms are not part of the actual AMP send.
 # Leave secondary AMP (OSC2 etc) real and its Swift call reaches d.set_param.
 handled=[]
 for aa,lab in ((0x10027b65c,'UI'),(0x10027189c,'notify'),(0x1000f0c6c,'linked filter release, separate'),(0x1000f47d8,'inactive non-ARP tail dispatch')):
  def skip(*_,lab=lab):
   handled.append(lab)
   e.uc.reg_write(UC_ARM64_REG_PC,e.uc.reg_read(UC_ARM64_REG_LR))
  e.uc.hook_add(UC_HOOK_CODE,skip,begin=aa,end=aa)
 fake_ui=e.calloc(0x700); field=0x180
 for off in (0x10082edb8,0x10082eeb0):e.w64(off,field)
 e.w64(cond+field,fake_ui)
 e.w64(fake_ui+0x60,0)
 cases=[]
 for x,link in ((0.,False),(.1,False),(.125,False),(.55,False),(1.,False),(raw,True)):
  calls.clear();handled.clear();e.wd(inst+0xf0,1. if link else 0.);e.wd(inst+0xf8,1. if link else 0.)
  e.call(0x1000f4168,floats=(f32(x),),regs={20:cond})
  expect=f32(.002+(24.-.002)*float(f32(x))**3.35)
  assert calls[:2]==[('Sub',15,expect),('OSC1',15,expect)],(x,calls,expect)
  assert [c for c in calls if c[0]=='Sub'][-1][1:]==(15,expect) or dict((slot,val) for whom,slot,val in calls if whom=='Sub')=={15:expect,14:0.}
  cases.append(dict(received=f32(x),release_physical_word=expect,link=link,actual_calls=list(calls),isolated_callbacks=list(handled)))
 assert not e.unknown
 return cases
