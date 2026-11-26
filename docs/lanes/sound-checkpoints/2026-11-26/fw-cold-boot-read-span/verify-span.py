import sys
from pathlib import Path
import subprocess,tempfile
o=Path(sys.argv[1] if len(sys.argv)>1 else 'editor-before-span.c').read_text()
s=Path(sys.argv[2] if len(sys.argv)>2 else 'editor-before-segment.c').read_text()
def take(s,a,b):
 return s[s.index(a):s.index(b)]
helper=take(s,'static void ed_b(', 'static void ed_str(')
pack=take(s,'static void ed_put7(', '/* Offset')
n=take(s,'static __attribute__((noinline)) int ed_read_area_impl','/* Sample/project')
a=take(o,'static int ed_read_area(', '/* Sample/project').replace('ed_read_area(', 'old_read_area(')
b=take(o,'    if (cmd == ED_BOOT_READ)', '    if (cmd == ED_BOOT_BEGIN)')
b=b[b.index('{')+1:b.rindex('}')]
c=take(s,'    if (cmd == ED_BOOT_READ)', '    if (cmd == ED_BOOT_BEGIN)')
c=c[c.index('{')+1:c.rindex('}')]
top=r"""
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#define ED_READ_MAX 224u
#define BOOT_PAY_OFF 32u
static uint8_t ed_out[732], mem[1900];
static unsigned ed_n;
static int shown;
static const uint8_t *boot_shown(void){return shown?mem:0;}
static uint32_t boot_u32(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
"""
main=r"""
int main(void){
 unsigned cases=0;
 for(unsigned i=0;i<sizeof mem;i++)mem[i]=(uint8_t)(i*25);
 for(unsigned l=0;l<=1200;l+=7){
  mem[12]=(uint8_t)l;mem[13]=(uint8_t)(l>>8);mem[14]=mem[15]=0;
  for(unsigned off=l>4?l-4:0;off<l+5;off++)
  for(unsigned k=0;k<256;k++)
  for(unsigned type=0;type<11;type++){
    uint8_t a[]={off&127,off>>7,0,k},old[732]; unsigned na=type%5, total;
    shown=type%2;
    memset(ed_out,0x55,sizeof ed_out);ed_n= k>251?729:0;
    int rc=-99,x=ed_n;
    if(type)oldboot(a,na);else rc=old_read_area(mem+BOOT_PAY_OFF,l,a);
    total=ed_n;memcpy(old,ed_out,sizeof old);
    memset(ed_out,0x55,sizeof ed_out);ed_n=x;
    if(type)newboot(a,na);else assert(rc==ed_read_area(mem+BOOT_PAY_OFF,l,a));
    assert(ed_n==total && !memcmp(old,ed_out,sizeof old));cases++;
  }
 }
 printf("%u exact editor/boot read and near-capacity responses\n",cases);
}
"""
src=top+helper+pack+a+n+'\nstatic void oldboot(const uint8_t *a, uint32_t na){'+b+'}\nstatic void newboot(const uint8_t *a, uint32_t na){'+c+'}\n'+main
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'w.c').write_text(src)
 subprocess.run(['cc','-O2','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=undefined',str(p/'w.c'),'-o',str(p/'w')],check=True)
 subprocess.run([str(p/'w')],check=True)
