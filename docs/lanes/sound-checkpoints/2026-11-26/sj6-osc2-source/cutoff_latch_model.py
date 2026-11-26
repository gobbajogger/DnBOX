"""Comparison-only model of verified public cutoff / live target separation."""
from dataclasses import dataclass,field
import math,struct

def f32(v:float)->float:
 return struct.unpack('<f',struct.pack('<f',v))[0]

@dataclass
class LiveCutoff:
    public: float
    base: float
    destination: int
    delay: float
    target: float=field(init=False)
    applied: float=field(init=False)
    running: bool=False
    keys: set[int]=field(default_factory=set)
    sampled_steps: int=0

    EPS = f32(.0005)

    def __post_init__(self):
        self.public=f32(self.public)
        self.base=f32(self.base)
        self.target=self.applied=self.public

    def set_public(self,raw:float)->float|None:
        old=self.public
        self.public=f32(min(1.,max(0.,raw)))
        if self.running:
            # Actual 0x1000eed70…a8: change relative to active live target.
            # Do not write the DSP directly while its prior relatch is armed.
            self.target=min(1.,max(0.,f32(f32(self.target-old)+self.public)))
            return None
        self.target=self.applied=self.public
        return self.applied

    def note(self,key:int,on:bool)->bool:
        if not on:
            self.keys.discard(key)
            return False
        first=not self.keys
        self.keys.add(key)
        if not (first and self.delay>=.01 and self.destination==7):return False
        if not self.running and abs(f32(self.base-self.public))<=self.EPS:
            self.target=self.applied=self.public
            return False
        if not self.running:self.applied=self.public
        self.target=self.base
        self.running=True
        self.sampled_steps=0
        return True

    def tick(self,dt:float)->float|None:
        if not self.running:return None
        if dt<0 or not math.isfinite(dt):raise ValueError(dt)
        inc=f32(1.-math.exp(-dt/.015)) if dt else 1.
        value=f32(self.applied+f32(f32(self.target-self.applied)*inc))
        self.applied=min(1.,max(0.,value))
        endpoint=self.applied
        if abs(f32(self.applied-self.target))<=self.EPS:
            self.applied=self.target
            # Current note restore's public baseline differs from its target.
            # The sender remains armed by source logic but redundant sends
            # can be suppressed. Keep elapsed updates without changing raw.
            if abs(f32(self.public-self.target))<=self.EPS:self.running=False
        self.sampled_steps+=1
        return endpoint
