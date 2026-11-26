from pathlib import Path
import sys,re
t=Path(sys.argv[1]).read_text()
a=t.index('                OWNER_PAGE(PT_PHASER,')
b=t.index('            }',a)
c=t.index('                OWNER_PAGE(PT_REVERB,QR(R_SJ6ROOM',b)
d=t.index('            }',c)
parts=[t[a:b],t[c:d]]
def change(s):
 s=re.sub(r'OWNER_PAGE\((PT_\w+),',r'{\1,{',s)
 s=re.sub(r'\);(?=\s*$)','}},',s,flags=re.M)
 # ONLY NONE tokens in this local old macro should be emitted as zero words.
 return 'static const owner_page_row pages[]={\n'+re.sub(r'\bNONE\b','0',s)+'};\nadd_owner_pages(emit,pages,sizeof pages/sizeof pages[0],b);\n'
new=t[:a]+change(parts[0])+t[b:c]+change(parts[1])+t[d:]
Path('ui.c').write_text(new)
# Extract actual descriptor operands with shared stable symbolic enum numbering.
# Native trial only compares call site/array shape, which is independent of these uint8 values.
ids=sorted(set(re.findall(r'\b(?:PT|ARP_F|R|SJ6)_[A-Z0-9_]+\b',''.join(parts))))
flags=sorted(set(re.findall(r'\bX0X_\w+\b',''.join(parts))))
preamble='''#include <stdint.h>
#define QR(k,c) ((uint16_t)((k)*256u+(c)))
#define J(k,c) QR(k,c)
#define NONE 0
typedef struct { uint8_t title; uint16_t ref[4]; } owner_page_row;
typedef struct {unsigned n; owner_page_row rows[40]; unsigned owner[40];} output;
#ifndef HOST
extern void add_owner_pages(output *,const owner_page_row *,unsigned,uint8_t);
#else
__attribute__((noinline)) void add_owner_pages(output *o,const owner_page_row *r,unsigned n,uint8_t b) {
 for(unsigned i=0;i<n;i++){o->rows[o->n]=r[i];o->owner[o->n++]=b;}
}
#endif
#define OWNER_PAGE(name, ...) do {static const owner_page_row row={name,{__VA_ARGS__}};add_owner_pages(emit,&row,1,b);}while(0)
'''+''.join(f'#define {v} {i+1}\n' for i,v in enumerate(ids))
for kind,src in [('before',parts),('after',list(map(change,parts)))]:
 Path(kind+'.c').write_text(preamble+''.join(f'void part_{i}(output *emit,uint8_t b){{\n{s}}}\n' for i,s in enumerate(src))+'''
#ifdef HOST
#include <stdio.h>
int main(){ for(unsigned b=0;b<2;b++){output o={0};part_0(&o,b);part_1(&o,b);
 for(unsigned i=0;i<o.n;i++)printf("%u %u %u %u %u %u\\n",o.owner[i],o.rows[i].title,o.rows[i].ref[0],o.rows[i].ref[1],o.rows[i].ref[2],o.rows[i].ref[3]);}return 0;}
#endif
''')
Path('flags').write_text(' '.join('-D'+p+'=1' for p in flags))
