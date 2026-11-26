from pathlib import Path
import re
p=Path('revsc-stock.c').read_text()
i=p.index('#define DEFAULT_SRATE');p=p[:i]+'#include "codec.h"\n'+p[i:]
p=p.replace('sizeof(SPFLOAT)','sizeof(room_word)')
p=p.replace('lp->buf[lp->writePos] = (SPFLOAT) ((n & 1 ? ainR : ainL)\n                                 - lp->filterState);','room_words(lp)[lp->writePos]=compress((n & 1 ? ainR : ainL)-lp->filterState);')
p,n=re.subn(r'\(SPFLOAT\)\s*\(?(lp->buf)\[([^]]+)\]\)?',r'expand(room_words(lp)[\2])',p)
assert n==8 and p.count('lp->buf[')==0
assert 'room_words(lp)[lp->writePos]=' in p
Path('revsc-quant.c').write_text(p)
