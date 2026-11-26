from pathlib import Path
import sys,re
p,o=map(Path,sys.argv[1:]);s=p.read_text()
k=s.index('#define DEFAULT_SRATE')
s=s[:k]+(Path(__file__).parent/'pair-store.inc').read_text()+'\n'+s[k:]
a='nBytes += (delay_line_max_samples(sr, iPitchMod, n) * (int) sizeof(SPFLOAT));'
assert s.count(a)==1;s=s.replace(a,'nBytes += (int)room_stride(delay_line_max_samples(sr,iPitchMod,n));')
a='memset(lp->buf, 0, sizeof(SPFLOAT) * lp->bufferSize);'
assert s.count(a)==1;s=s.replace(a,'memset(lp->buf,0,room_stride(lp->bufferSize));')
a='''lp->buf[lp->writePos] = (SPFLOAT) ((n & 1 ? ainR : ainL)
                                 - lp->filterState);'''
assert s.count(a)==1;s=s.replace(a,'''room_put(lp,lp->writePos,(SPFLOAT)((n&1?ainR:ainL)-lp->filterState));''')
s,n=re.subn(r'\(SPFLOAT\)\s*\(lp->buf\[([^\]]+)\]\)',r'room_read(lp,\1)',s)
s,m=re.subn(r'\(SPFLOAT\)\s*lp->buf\[([^\]]+)\]',r'room_read(lp,\1)',s)
assert n==m==4,(n,m)
o.write_text(s)
