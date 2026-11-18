#!/usr/bin/env python3
"""Exercise real production loop publication and preview with the real decoder."""
from pathlib import Path
import re,subprocess,tempfile
root=Path(__file__).resolve().parents[2];s=(root/'firmware/src/app/engine.c').read_text()
def fn(name):
 m=re.search(r'(?:X0X_COLD )?(?:static )?(?:int|void) '+name+r'\([^;]*?\)\s*\{',s,re.S);assert m,name
 i=s.index('{',m.start());d=1;j=i+1
 masked=re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"',lambda m:' '*len(m[0]),s,flags=re.S)
 while d:d+=(masked[j]=='{')-(masked[j]=='}');j+=1
 return s[m.start():j]
code='''#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#define X0X_IMA_TABLES_DEFINE
#include "seq/sequencer.h"
#include "dsp/breaks.c"
static unsigned cq_w,cq_r;static unsigned brk_preview_pattern;static seq_t seq;static breaks_t brk;static int brk_preview_slot=-1,project_hold,nloops=3;
static uint8_t loop_slot[3]={0,1,2},loop_zone[3]={0,0,0};
static uint8_t data[3][512];
static void scan_loops(void){}
static int builtin_count(void){return 0;}
static const brk_loop_t *brk_builtin(int x){(void)x;return 0;}
const uint8_t *plat_slot(int s,int z,uint32_t*n,uint32_t*r,char name[9]){(void)z;name[0]=0;*n=1024;*r=22050;return data[s];}
static uint8_t loop_bars(uint32_t n,uint32_t r){(void)n;(void)r;return 1;}
static int engine_brk_nslots(void){return nloops;}
#define C_BRK 1
void engine_brk_preview_end(void);
static uint32_t engine_brk_held(void){return brk.slices;}
static void engine_brk_live(int key,int down){if(key>=32){if(down)breaks_slice_on(&brk,key-32);else breaks_live(&brk,key-32,0);}else breaks_live(&brk,key,down);}
'''
code+='\nstatic void cq_put(int type,int key,int down,int tag){(void)type;(void)tag;engine_brk_live(key,down); }\n'
code+='\n'+fn('engine_brk_loops')+'\n'+fn('engine_brk_preview')+'\n'+fn('engine_brk_preview_end')
code+='''
int main(void){pattern_t patterns[NPAT];memset(patterns,0,sizeof patterns);seq.pat=patterns;patterns[0].brk.slot_a=0;patterns[0].brk.slot_b=1;
 memset(data[0],0x10,512);memset(data[1],0x32,512);memset(data[2],0x45,512);breaks_init(&brk);engine_brk_loops();
 pattern_t before=patterns[0];assert(engine_brk_preview(2));assert(!memcmp(&before,&patterns[0],sizeof before));assert(breaks_count(&brk,0)==1);assert(breaks_bank(&brk,0)->adpcm==data[2]);assert(breaks_bank(&brk,1)->adpcm==data[1]);
 float out[256],peak=0;for(int j=0;j<8;j++){breaks_render(&brk,out,256);for(int i=0;i<256;i++){assert(isfinite(out[i]));if(fabsf(out[i])>peak)peak=fabsf(out[i]);}}assert(peak>0.00001f);
 engine_brk_preview_end();breaks_render(&brk,out,256);breaks_flush(&brk);assert(!memcmp(&before,&patterns[0],sizeof before));assert(brk_preview_slot==-1);assert(breaks_bank(&brk,0)->adpcm==data[0]);assert(breaks_bank(&brk,1)->adpcm==data[1]);
 assert(!engine_brk_preview(-1));assert(!engine_brk_preview(3));project_hold=1;assert(!engine_brk_preview(2));project_hold=0;breaks_slice_on(&brk,1);assert(!engine_brk_preview(2));
 printf("BREAK preview actual decoder peak=%f pattern unchanged, banks restored PASS\\n",peak);return 0;}
'''
with tempfile.TemporaryDirectory() as t:
 p=Path(t);(p/'test.c').write_text(code)
 subprocess.run(['cc','-std=c99','-O1','-DX0X_HOST','-Wall','-Wextra','-Werror','-I'+str(root/'firmware/src'),str(p/'test.c'),'-lm','-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
