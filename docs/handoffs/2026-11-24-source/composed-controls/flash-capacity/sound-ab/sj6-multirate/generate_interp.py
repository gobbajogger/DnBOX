"""Emit the float32 equivalent of stream_new's interpolation coefficients."""
import math, struct,sys
phases=int(sys.argv[1]) if len(sys.argv)>1 else 5
def s(x):return 1. if abs(x)<1e-10 else math.sin(math.pi*x)/(math.pi*x)
def f(x):return struct.unpack('<f',struct.pack('<f',x))[0]
print('{')
for phase in range(phases):
    lights=[(j-3-phase/phases) for j in range(8)]
    values=[s(n)*s(n/4) for n in lights]; norm=f(sum(values))
    print('    {'+', '.join(f(f(n)/norm).hex()+'f' for n in values)+'},')
print('}')
