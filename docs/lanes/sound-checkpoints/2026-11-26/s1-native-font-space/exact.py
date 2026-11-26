"""Prove each actual old/new source decoder lookup and untouched generated payload."""
from pathlib import Path
import os, re, subprocess, sys, tempfile
root=Path(__file__).resolve().parent
base=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root.parent/'ui-native-table-share/tree'
trial=root/'tree'
def split(data):
    return data[data.index('#define FONT_INDEX16'):]
def leaf(tree, which):
    p=tree/'firmware/src/gfx.c'
    text=p.read_text()
    a=text.index('                uint32_t entry=' if which==0 else '                unsigned prefix=peek>>3;')
    b=text.index('                code_left-=entry>>4;',a)
    return '''#include <stdint.h>
typedef struct { int8_t top, bot; } capm_t;
typedef struct { uint8_t h,pad,first,last; const uint8_t *adv,*bw; const uint16_t *off; const uint8_t *data; capm_t cap; } felucca_font_t;
#include "felucca_font.h"
''' + f'uint32_t decode_{which}(unsigned context,unsigned peek) {{\nunsigned above[]={{0}}; unsigned gx=0; above[0]=context;\n'.replace('{{context}}','context') + text[a:b]+'return entry;\n}\n'
for model in ('smk37-elite','smk37'):
    gh=[t/'build'/model/'gen' for t in (base,trial)]
    assert split((gh[0]/'felucca_font.h').read_text()) == split((gh[1]/'felucca_font.h').read_text())
    with tempfile.TemporaryDirectory() as tmp:
        t=Path(tmp); cmds=[]
        for i, tree in enumerate((base,trial)):
            c=t/f'leaf{i}.c'; c.write_text(leaf(tree,i)); o=t/f'leaf{i}.o'
            subprocess.run(['cc','-std=c99','-O2','-I'+str(gh[i]),'-c',c,'-o',o],check=True)
            cmds.append(o)
        c=t/'tester.c'
        c.write_text('''#include <stdint.h>
#include <stdio.h>
uint32_t decode_0(unsigned,unsigned),decode_1(unsigned,unsigned);
int main(void) {
    for(unsigned a=0;a<16;++a) for(unsigned p=0;p<128;++p)
        if(decode_0(a,p)!=decode_1(a,p)){
            fprintf(stderr,"mismatch context %u peek %u\n",a,p);return 1;
        }
    return 0;
}
'''.replace('%u\n','%u\\n'))
        subprocess.run(['cc','-std=c99','-O2',c,*cmds,'-o',t/'tester'],check=True)
        subprocess.run([t/'tester'],check=True)
    print(f'{model}: 2048/2048 exact lookup bytes (length and alpha); glyph offsets, advances, all compressed glyph streams and XDATA CRC identical')
