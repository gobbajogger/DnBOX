from pathlib import Path
import sys
d=Path('firmware/src/dsp')
h=d/'juno6.h'; c=d/'juno6.c'
s=h.read_text(); k=' int16_t delay[JUNO6_CHORUS_SAMPLES];'
assert s.count(k)==1
if sys.argv[1]=='1':
    s=s.replace(k,k+'\n blockmeta classic_blocks;')
    s='#include "block-history.h"\n'+s
    h.write_text(s)
    s=c.read_text()
    a='return ((float)c->delay[a] * (1 - frac) + (float)c->delay[b] * frac) / 32768.0f;'
    b='return blockread(c->delay,&c->classic_blocks,write,a)*(1-frac)+blockread(c->delay,&c->classic_blocks,write,b)*frac;'
    assert s.count(a)==1;s=s.replace(a,b)
    a='c->delay[write] = (int16_t)clampf(in * 32767, -32768, 32767);'
    b='unsigned ignored=0; blockwrite(c->delay,&c->classic_blocks,write,in,&ignored);'
    assert s.count(a)==1;s=s.replace(a,b)
    c.write_text(s)
