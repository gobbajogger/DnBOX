#!/usr/bin/env python3
"""Run actual pending BREAK chooser functions with explicit UI/engine boundaries."""
from pathlib import Path
import re,subprocess,tempfile,os
root=Path(__file__).resolve().parents[2]
s=(root/'firmware/src/app/ui.c').read_text()
def function(name):
 m=re.search(r'(?:static )?(?:int|void) '+name+r'\([^;]*?\)\s*\{',s,re.S)
 assert m,'missing production '+name
 start=m.start();i=s.index('{',start);depth=1;j=i+1
 masked=re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"',lambda m:' '*len(m[0]),s,flags=re.S)
 while depth:depth+=(masked[j]=='{')-(masked[j]=='}');j+=1
 return s[start:j]
code='''#include <assert.h>
#include <stdint.h>
#include <string.h>
#define R_BRKSLOT 7
#define PART_BRK 4
#define PR(k,a,b,c) ((pref_t){k,a,b,c})
typedef struct {int kind,a,b,c;} pref_t;
static struct {uint8_t brk_choice_on,brk_choice_bank,brk_choice_slot,brk_choice_pat; uint8_t outline_ok;} ui;
static struct {uint8_t ppat[5];} seq;
static uint8_t selected[2]={1,2};static int audition=-1,commits,mark_count,available=8,refused;
static int engine_brk_nslots(void){return available;}
static int engine_brk_preview(int slot){if(refused)return 0;audition=slot;return 1;}
static void engine_brk_preview_end(void){audition=-1;}
static int engine_brk_preview_active(void){return audition>=0;}
static int pref_get(pref_t r){return selected[r.a];}
static void pref_set_impl(pref_t r,int v,int keyboard){(void)keyboard;selected[r.a]=v;commits++;}
static void mark_dirty(void){mark_count++;}
static void say(const char*a,const char*b){(void)a;(void)b;}
'''
code+='\n'+function('brk_choice_cancel')+'\n'+function('brk_choice_turn')+'\n'+function('brk_choice_confirm')
code+='''
int main(void){pref_t a=PR(R_BRKSLOT,0,0,0),b=PR(R_BRKSLOT,1,0,0);
 assert(brk_choice_turn(a,1));assert(audition==2&&selected[0]==1&&commits==0&&mark_count==0);
 assert(brk_choice_turn(a,1));assert(audition==3&&selected[0]==1);
 brk_choice_cancel();assert(audition==-1&&!ui.brk_choice_on&&selected[0]==1&&commits==0);
 assert(brk_choice_turn(b,1));assert(audition==3&&selected[1]==2);assert(brk_choice_confirm());assert(selected[1]==3&&selected[0]==1&&commits==1&&mark_count==1&&audition==-1);
 assert(!brk_choice_confirm());assert(commits==1);
 assert(brk_choice_turn(a,1));seq.ppat[4]++;assert(!brk_choice_confirm());assert(selected[0]==1&&commits==1&&!ui.brk_choice_on);
 refused=1;assert(!brk_choice_turn(a,1));assert(!ui.brk_choice_on&&commits==1);refused=0;
 available=0;assert(!brk_choice_turn(a,1));assert(!ui.brk_choice_on);
 return 0;}
'''
with tempfile.TemporaryDirectory() as t:
 p=Path(t);(p/'test.c').write_text(code)
 subprocess.run(['cc','-std=c99','-Wall','-Wextra','-Werror',str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('BREAK chooser production control: preview/cancel/confirm/stale/refusal PASS')
