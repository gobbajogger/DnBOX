import hashlib,sys,re
from pathlib import Path
s=Path(sys.argv[1]).read_text()
assert hashlib.sha256(s.encode()).hexdigest()=='f13e4cf1619ccd630848e099517615e4b2942c12ef117f98d70dfda5700cb3ce'
out=Path(sys.argv[2])
pfx='''
#ifndef LAYOUT
#define LAYOUT 0
#endif
/* Fixed early pair. Physical line 6/7 are the shortest of each polarity. */
static int admitted(unsigned n){return LAYOUT==0||n>=6;}
#if LAYOUT==2
extern uint32_t pack_float(float,unsigned,unsigned,unsigned);
extern float unpack_float(uint32_t,unsigned,unsigned,unsigned);
#define LOAD_AT(lp,i) unpack_float(((uint8_t*)(lp)->buf)[i],4,3,13)
#define STORE_AT(lp,i,val) (((uint8_t*)(lp)->buf)[i]=(uint8_t)pack_float(val,4,3,13))
#else
#define LOAD_AT(lp,i) ((lp)->buf[i])
#define STORE_AT(lp,i,val) ((lp)->buf[i]=(val))
#endif
'''
s=s.replace('#define DEFAULT_SRATE',pfx+'\n#define DEFAULT_SRATE')
a='nBytes += delay_line_bytes_alloc(sp->sr, 1, i);'
assert s.count(a)==2
s=s.replace(a,'if(admitted(i)) '+a)
a='p->delayLines[i].buf = (p->aux.ptr) + nBytes;'
assert s.count(a)==1
s=s.replace(a,'if(!admitted(i)){memset(&p->delayLines[i],0,sizeof p->delayLines[i]);continue;}\n        '+a)
a='nBytes += (delay_line_max_samples(sr, iPitchMod, n) * (int) sizeof(SPFLOAT));'
assert s.count(a)==1
s=s.replace(a,'nBytes += LAYOUT==2 ? (delay_line_max_samples(sr, iPitchMod, n)+3)&~3 : (delay_line_max_samples(sr, iPitchMod, n) * (int) sizeof(SPFLOAT));')
s=s.replace('sizeof(SPFLOAT) * lp->bufferSize','(LAYOUT==2 ? 1 : sizeof(SPFLOAT)) * lp->bufferSize')
a='lp = &p->delayLines[n];'
assert s.count(a)==1
s=s.replace(a,'if(!admitted(n)) continue;\n        '+a)
a='''lp->buf[lp->writePos] = (SPFLOAT) ((n & 1 ? ainR : ainL)
                                 - lp->filterState);'''
assert s.count(a)==1
s=s.replace(a,'STORE_AT(lp,lp->writePos,(SPFLOAT)((n & 1 ? ainR : ainL) - lp->filterState));')
s,n=re.subn(r'\(SPFLOAT\) \(lp->buf\[(readPos(?: [+-] [12])?)\]\)',r'LOAD_AT(lp,\1)',s)
s,n2=re.subn(r'\(SPFLOAT\) lp->buf\[readPos\]','LOAD_AT(lp,readPos)',s)
assert n==n2==4 and 'lp->buf[' not in s
out.write_text(s)
