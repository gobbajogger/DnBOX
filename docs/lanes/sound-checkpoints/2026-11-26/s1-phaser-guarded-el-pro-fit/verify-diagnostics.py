from pathlib import Path
import re,tempfile,subprocess,sys
d=Path(__file__).resolve().parent
base=Path(sys.argv[1]).read_text()
new=Path(sys.argv[2]).read_text()
def extract(s,sig):
    a=s.index(sig); b=s.index('{',a);e=b+1;level=1
    while level:
        level+=(s[e]=='{')-(s[e]=='}');e+=1
    return s[a:e]+'\n'
a=base.index('static void do_undo(int redo)');b=base.index('    song_structure_revision++;',a)
original=base[a:b]
a=new.index('static void do_undo(int redo)');b=new.index('    song_structure_revision++;',a)
patched=new[a:b]
pat=r'((?:undo_bounded_field|ui_undo_span)\([\s\S]*?,)"([A-Z0-9 /-]+)"(\)\)return;)'
expanded,n=re.subn(pat,r'\1"UNDO \2 INVALID"\3',patched);assert n==25
expanded,m=re.subn(r'undo_field_invalid\("([A-Z0-9 /-]+)"\)',r'ui_undo_invalid("UNDO \1 INVALID")',expanded)
expanded,k=re.subn(r'(ui_undo_check\([^,]+,[^,]+,)"([A-Z0-9 /-]+)"',r'\1"UNDO WAIT \2"',expanded)
assert expanded==original,(n,m,k)

good=sorted(set(re.findall(r'"UNDO ([A-Z0-9 /-]+) INVALID"',original)))
wait=sorted(set(re.findall(r'"UNDO WAIT ([A-Z0-9 /-]+)"',original)))
old_wait=extract(base,'static __attribute__((noinline)) int ui_undo_check(').replace('ui_undo_check','old_check')
new_wait=extract(new,'static __attribute__((noinline)) int ui_undo_check(')
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp)
 src='''#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#define X0X_COLD
static struct {char msg[64];uint32_t msg_until;} ui;
static uint32_t now, ready_calls, ready_mask, ready_value, trace, undo_bass_mask;
static uint32_t plat_ms(void){trace=trace*257+3;return now;}
static int ready(unsigned x){++ready_calls;ready_mask=x;trace=trace*257+5;return (int)ready_value;}
static void engine_project_hold(int x){assert(x==0);trace=trace*257+7;}
static void engine_bass_edit_hold(int x){assert(x==0);trace=trace*257+11;}
'''
 src+=extract(base,'static char *put_s(')+extract(base,'static void say(')
 src+=extract(new,'static __attribute__((noinline)) X0X_COLD void undo_field_message(')+extract(new,'static __attribute__((noinline)) X0X_COLD void undo_field_invalid(')
 src+=extract(base,'static __attribute__((noinline)) void ui_undo_invalid(')+old_wait+new_wait
 src+='''typedef struct{uint32_t trace,until,calls,mask;int result;char message[64];} result;
static void pre(void){trace=ready_calls=ready_mask=0;memset(ui.msg,0x7e,sizeof ui.msg);ui.msg_until=0;}
static result witness(int ret){
 result r;memset(&r,0,sizeof r);
 r.trace=trace;r.until=ui.msg_until;r.calls=ready_calls;r.mask=ready_mask;r.result=ret;memcpy(r.message,ui.msg,sizeof r.message);return r;
}
static const char *bad[] = {'''+','.join('"'+g+'"' for g in good)+'''};
static const char *waits[] = {'''+','.join('"'+g+'"' for g in wait)+'''};
int main(void){
 unsigned checks=0;
 const uint32_t times[]={0,1300,0xfffffffeu};
 for(unsigned t=0;t<3;++t){
  now=times[t];
  for(unsigned i=0;i<sizeof bad/sizeof bad[0];++i){
   char str[64];snprintf(str,sizeof str,"UNDO %s INVALID",bad[i]);
   pre();ui_undo_invalid(str);result a=witness(0);
   pre();undo_field_invalid(bad[i]);result b=witness(0);
   assert(!memcmp(&a,&b,sizeof a));++checks;
  }
  for(unsigned i=0;i<sizeof waits/sizeof waits[0];++i){
   char str[64];snprintf(str,sizeof str,"UNDO WAIT %s",waits[i]);
   for(unsigned c=0;c<4;++c)for(undo_bass_mask=0;undo_bass_mask<4;++undo_bass_mask)
     for(ready_value=0;ready_value<2;++ready_value){
       pre();int ret=old_check(c,ready,str);result a=witness(ret);
       pre();ret=ui_undo_check(c,ready,waits[i]);result b=witness(ret);
       assert(!memcmp(&a,&b,sizeof a));++checks;
     }
  }
 }
 printf("%u exact diagnostics fixtures incl. wrapped expiry, order of holds/ready/audio and unchanged full message buffer\\n",checks);
}
'''
 (p/'test.c').write_text(src)
 subprocess.run(['cc','-std=c99','-O2',p/'test.c','-o',p/'test'],check=True)
 subprocess.run([p/'test'],check=True)
print(f'Full branch body reconstructed byte-identical at {n} bounded/span, {m} direct-invalid and {k} wait callsites.')
