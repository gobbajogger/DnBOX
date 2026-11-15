"""Own integer/Float literal laws, not original instruction execution.
An externally supplied64-bit entropy tape is a test input, not the iOS RNG.
"""
import struct,json
F=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
class Tape:
 def __init__(self,words):self.words=iter(words);self.requests=0
 def take(self):
  x=next(self.words)
  if type(x)is not int or not 0<=x<1<<64:raise ValueError('expected uint64 entropy word')
  self.requests+=1;return x

def bounded(tape,bound,width):
 if not 0<bound<1<<width:raise ValueError('bound outside observed nonzero domain')
 mask=(1<<width)-1;threshold=((1<<width)-bound)%bound
 while True:
  x=tape.take()&mask;p=x*bound
  if (p&mask)>=threshold:return p>>width

def amplitude(tape):
 x=bounded(tape,(1<<24)+1,32)
 return 1. if x==(1<<24) else F(F(F(x/(1<<24))*2)-1)

def table12(tape):
 # Both discarded initialization draws are externally observable entropy consumption.
 amplitude(tape);bounded(tape,207,64)
 table=[];remaining=0;value=0.
 for i in range(256):
  if remaining==0:
   value=amplitude(tape);remaining=50+bounded(tape,207,64)
  table.append(value);remaining-=1
 return table

def tests():
 n=0
 for width in (32,64):
  for bound in (1,2,3,207,(1<<24)+1):
   t=Tape([(1<<64)-1]);assert bounded(t,bound,width)==bound-1;n+=1
   if ((1<<width)-bound)%bound:
    t=Tape([0,(1<<64)-1]);assert bounded(t,bound,width)==bound-1 and t.requests==2;n+=1
 # A fullscale amplitude + maximumduration uses4requests, including discarded pair.
 t=Tape([(1<<64)-1]*10);w=table12(t);assert len(w)==256 and w==[1.]*256 and t.requests==4;n+=1
 # Force duration50: smallest64-bit multiplicand that yields a nonrejected low product
 # with highhalf0. Amplitude maxdraw remains1. No shared PRNG is invented.
 small=1;assert (small*207)&((1<<64)-1)>=((1<<64)-207)%207
 tape=[(1<<64)-1,small]*8;t=Tape(tape);w=table12(t);assert w==[1.]*256 and t.requests==14;n+=1
 # Low32bits define Float draw; high32bits still belong to consumed8-byte request.
 for high in (0,1,0xffffffff):
  t=Tape([(high<<32)|0xffffffff]);assert amplitude(t)==1.;n+=1
 # Exhaustion is explicit, never replaced by invented silence/default entropy.
 try:table12(Tape([]));raise AssertionError('missing entropy accepted')
 except StopIteration:n+=1
 return dict(status='PASS',checks=n,scope='Own supplied-entropy literal laws; not iOS random trace or original execution')
if __name__=='__main__':print(json.dumps(tests(),indent=2))
