import sys
from pathlib import Path
import subprocess,tempfile
orig=Path(sys.argv[1] if len(sys.argv)>1 else 'storage-before-select.c').read_text()
cand=Path(sys.argv[2] if len(sys.argv)>2 else 'tree/firmware/src/storage.c').read_text()
def sub(s,a,b):return s[s.index(a):s.index(b)]
old=sub(orig,'static int st_current(', '/* Body CRC')+sub(orig,'static int st_pick(', '/* Length, -1')
old=old.replace('st_current(', 'old_current(').replace('st_pick(', 'old_pick(')
new=sub(cand,'static __attribute__((noinline)) int st_choose(', '/* Length, -1')
src=r"""
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
typedef struct {uint32_t seq,len,a,b,c,d;} st_hdr_t;
static st_hdr_t HDR[2];
static int H[2],P[2], trace[5], n;
static int st_head(uint32_t obj,uint32_t copy,st_hdr_t *h){
 (void)obj; trace[n++]=10+copy; if(!H[copy])*h=HDR[copy];return H[copy];}
static int st_body(uint32_t obj,uint32_t copy,const st_hdr_t *h){
 (void)obj;trace[n++]=20+copy; assert(!memcmp(h,&HDR[copy],sizeof *h));return P[copy];}
static int st_body_ok(uint32_t obj,uint32_t copy,const st_hdr_t *h){
 (void)obj;trace[n++]=30+copy; assert(!memcmp(h,&HDR[copy],sizeof *h));return P[copy]==0;}
"""+old+new+r"""
int main(void){
 int t[5], tn, a,b,count=0;
 for(H[0]=-2;H[0]<=0;H[0]++) for(H[1]=-2;H[1]<=0;H[1]++)
 for(P[0]=-2;P[0]<=0;P[0]++) for(P[1]=-2;P[1]<=0;P[1]++)
 for(unsigned s=0;s<3;s++){
   HDR[0]=(st_hdr_t){1,17,1,2,3,4};
   HDR[1]=(st_hdr_t){s,22,5,6,7,8};st_hdr_t x={0},y={0}; uint32_t p=53,q=53;
   n=0;a=old_current(0,&x);tn=n;memcpy(t,trace,n*sizeof(int));
   n=0;b=st_current(0,&y);assert(a==b&&n==tn&&!memcmp(t,trace,n*sizeof(int))&&!memcmp(&x,&y,sizeof x));
   n=0;a=old_pick(0,&p);tn=n;memcpy(t,trace,n*sizeof(int));
   n=0;b=st_pick(0,&q);assert(a==b&&n==tn&&!memcmp(t,trace,n*sizeof(int))&&p==q);
   count+=2;
 }
 printf("%d exact A/B reader/writer ordering and failure checks\n",count);
}
"""
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(src)
 subprocess.run(['cc','-O2','-Wall','-Wextra','-Werror','-std=c11','-fsanitize=undefined',str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
