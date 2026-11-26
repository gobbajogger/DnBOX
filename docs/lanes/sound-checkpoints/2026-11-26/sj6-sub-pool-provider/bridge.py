from pathlib import Path
import ctypes as c, struct
import numpy as np
from unicorn import UC_HOOK_CODE
from unicorn.arm64_const import UC_ARM64_REG_PC,UC_ARM64_REG_LR
class Media(c.Structure):
 _fields_=[('pair',c.c_void_p),('ctx',c.c_void_p),('frames',c.c_uint),('channels',c.c_uint),('loop',c.c_uint),
            ('start',c.c_float),('end',c.c_float),('first',c.c_float),('last',c.c_float)]
class Pcm(c.Structure):
 _fields_=[('data',c.c_void_p),('frames',c.c_uint),('channels',c.c_uint)]
class Owner(c.Structure):
 _fields_=[('media',c.c_void_p),('cursor',c.c_double),('gate',c.c_uint),('active',c.c_uint)]
class Bridge:
 def __init__(self,file):
    d=self.lib=c.CDLL(str(file))
    d.sub_commit.argtypes=[c.POINTER(Owner),c.POINTER(Media)]
    d.sub_gate.argtypes=[c.POINTER(Owner),c.c_uint]
    d.sub_frame.argtypes=[c.POINTER(Owner),c.c_double,c.c_double,c.c_float,c.POINTER(c.c_float)]
    self.pair=c.cast(d.sub_pcm16_pair,c.c_void_p)
    self.calls=self.commits=self.wraps=self.gates=0
    self.entries=[]
 def install(self,e,frame,advance,event,regions,pack):
    d=self.lib
    # Read-only PCM buffer provided directly by the existing authorized decoded asset;
    # the adapter makes no sample copy and never writes through this buffer.
    table={}
    for g in regions:
      rate,med=pack[g['sample'].replace('\\','/').split('/')[-1]]
      assert rate==44100 and med.dtype==np.int16 and med.flags.c_contiguous
      pcm=Pcm(med.ctypes.data, len(med),1)
      for k in range(int(g['lokey']),int(g['hikey'])+1):
        assert k not in table
        table[k]=pcm
    owners={};out=(c.c_float*2)()
    def hook(*_):
      voice,zone=e.x(0),e.x(1)
      assert zone and voice==e.x(24)+8 and zone==e.r64(voice+0x20)
      key=e.r32(voice+0x4c0); at=frame()+advance(key)
      media,channels,n=(e.r64(zone),e.r32(zone+0xc),e.r32(zone+0x10))
      pcm=table[key]
      assert media and channels==1 and n==pcm.frames
      gate=e.rd(voice,1)[0]; assert gate in (0,1)
      start_current,r,ratio=[e.rd_(voice+a) for a in (8,16,24)]
      if voice not in owners or owners[voice][2]!=key:
        assert (key,86) in event[at] and start_current==0
        m=Media(self.pair,c.addressof(pcm),n,1,e.rd(zone+0x1c,1)[0],0,e.rf(zone+0x18),e.rf(zone+0x20),e.rf(zone+0x24))
        o=Owner()
        assert d.sub_commit(c.byref(o),c.byref(m))
        self.entries.append([key,at,r,ratio])
        owners[voice]=(o,m,key,zone)
        self.commits+=1
      o,m,k,ptr=owners[voice]
      assert key==k and ptr==zone and struct.pack('<d',o.cursor)==struct.pack('<d',start_current)
      self.gates+=o.gate!=gate
      assert d.sub_gate(c.byref(o),gate)
      assert d.sub_frame(c.byref(o),r,ratio,e.s(0),out)==1
      self.wraps+=o.cursor<start_current
      e.wr(e.x(3),struct.pack('<f',out[0]));e.wr(e.x(4),struct.pack('<f',out[1]))
      e.wd(voice+8,o.cursor);e.setx(0,0)
      e.uc.reg_write(UC_ARM64_REG_PC,e.uc.reg_read(UC_ARM64_REG_LR))
      self.calls+=1
    e.uc.hook_add(UC_HOOK_CODE,hook,begin=0x1003b8224,end=0x1003b8224)
