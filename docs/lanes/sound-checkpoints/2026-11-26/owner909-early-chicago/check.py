"""Compile actual patched C claim and fallback bodies against a small host harness."""
import pathlib,re,subprocess,sys,tempfile
d=pathlib.Path(__file__).resolve().parent
src=pathlib.Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(dir=d) as td:
 p=pathlib.Path(td); target=p/'engine.c'; target.write_bytes(src.read_bytes())
 subprocess.run(['patch','--fuzz=0','-s',str(target),str(d/'diagnostic.patch')],check=True)
 t=target.read_text()
 def extract(sig):
  i=t.index(sig+'\n{'); end=t.index('\n}',i)+3;return t[i:end]
 text=r'''
#include <stdint.h>
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
enum {JB_909,JB_808,JB_A,JB_B,JB_BRK,JB_N};
static uint32_t early_909_claim,early_909_partial_fault,eng_c1_mask,eng_c1_next,eng_stage,
   eng_808_body,eng_c1_waited,brk_midi_count,eng_brk_mark,calls;
static void eng_sync(void){}
static int plat_cpu2_wait(void){return -1;}
static int run_job(int j,int core){assert(!j&&!core);calls++;eng_stage=1;return 1;}
static jmp_buf tr;
#define __builtin_trap() longjmp(tr,1)
#define X0X_DIAG_OWNER909_EARLY_CHICAGO 1
'''+extract('static void early_909_check_halted(void)')+'\n'+extract('static void c1_take_over(uint32_t s)')+r'''
int main(void){
 for(unsigned c=0;c<3;c++)for(unsigned body=0;body<2;body++)for(unsigned stage=0;stage<2;stage++){
  eng_c1_mask=1;eng_c1_next=0;eng_c1_waited=0;
  early_909_claim=c;eng_808_body=body;eng_stage=stage;early_909_partial_fault=0;calls=0;
  if(!setjmp(tr))c1_take_over(JB_N+1u);
  unsigned should_trap=(early_909_claim==1&&!eng_808_body)||(early_909_claim==2&&!stage);
  assert(early_909_partial_fault==(should_trap?early_909_claim:0));
  assert(calls==(stage||should_trap?0:1));
 }
 puts("12 actual patched C fallback cases: unclaimed can replay; claimed unfinished traps; completed cannot replay");
}
'''
 c=p/'test.c';c.write_text(text)
 exe=p/'test'
 subprocess.run(['cc','-std=c11','-O0','-Wall','-Werror',str(c),'-o',str(exe)],check=True)
 subprocess.run([exe],check=True)
