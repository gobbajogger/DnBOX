from pathlib import Path
import subprocess,tempfile,re,sys
s=Path(sys.argv[1] if len(sys.argv)>1 else 'tree/firmware/src/app/editor.c').read_text()
helper=s[s.index('static void ed_b('):s.index('static void ed_str(')]
src=r'''
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t ed_out[257];
static unsigned ed_n;
'''+helper+r'''
static uint32_t oldg(const uint8_t *a,int n) {
  if(n==2)return (uint32_t)a[0] | (uint32_t)a[1]<<7;
  if(n==3)return (uint32_t)a[0] | (uint32_t)a[1]<<7 | (uint32_t)a[2]<<14;
  return (uint32_t)a[0] | (uint32_t)a[1]<<7 | (uint32_t)a[2]<<14 |
    (uint32_t)a[3]<<21 | (uint32_t)a[4]<<28;
}
static uint32_t next(uint32_t x) {x ^= x<<13; x ^= x>>17; return x^x<<5;}
int main(void) {
  unsigned cases=0;
  for(uint32_t k=0;k<1u<<24;k++) {
    uint8_t a[5]={k,(k>>8),(k>>16),0,0};
    assert(oldg(a,3)==ed_g(a,3));
    ++cases;
  }
  uint32_t r=1;
  int ns[]={2,3,5};
  for(unsigned k=0;k<65536;k++) {
    r=next(r);
    uint32_t a[5];
    for(unsigned j=0;j<5;j++) a[j]=r>>(7*j);
    for(unsigned i=0;i<3;i++) {
      int n=ns[i];
      uint8_t b[5]={r,r>>8,r>>16,r>>24,k};
      assert(oldg(b,n)==ed_g(b,n));
      for(unsigned pos=250;pos<259;pos++) {
        uint8_t expect[sizeof ed_out];
        memset(ed_out,0xC3,sizeof ed_out);
        memcpy(expect,ed_out,sizeof expect);
        unsigned want=ed_n=pos;
        for(int j=0;j<n;j++) if(want<sizeof expect-1)expect[want++]=a[j]&127;
        ed_u(r,n);
        assert(want==ed_n && !memcmp(expect,ed_out,sizeof ed_out));
        ++cases;
      }
    }
  }
  printf("%u byte read/write checks passed\n",cases);
}
'''
with tempfile.TemporaryDirectory() as d:
    p=Path(d)
    (p/'test.c').write_text(src)
    subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-O2','-fsanitize=undefined',str(p/'test.c'),'-o',str(p/'test')],check=True)
    subprocess.run([str(p/'test')],check=True)
