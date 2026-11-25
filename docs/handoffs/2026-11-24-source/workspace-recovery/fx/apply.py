#!/usr/bin/env python3
"""Apply the S1 b20+b46 native delta on an already-composed typed carrier."""
import re,sys
from pathlib import Path
root=Path(sys.argv[1]); out={}
def read(p): return (root/p).read_text()
def one(s,a,b):
    if s.count(a)!=1: raise ValueError(f'{s.count(a)} anchors: {a[:160]}')
    return s.replace(a,b,1)
def clone(s, pat):
    m=re.search(pat,s,re.M)
    if not m: raise ValueError(pat)
    x=m[0];y=(x.replace('s1_dict','s1_cwg').replace('s1_typed_dict','s1_typed_cwg')
        .replace('S1_DICT','S1_CWG').replace('R_S1DICT','R_S1CWG').replace('undo_dict','undo_cwg')
        .replace('dict_off','cwg_off').replace('IN CUTOFF','WET MAKEUP')
        .replace('X0X_S1_DELAY_TRACK_TEST','DNBOX_TYPED_CONTROLS_NATIVE_TEST'))
    return one(s,x,x+y)
A='firmware/src/app/'
p=A+'typed_controls_trial.h'
s=read(p)
s=one(s,'X0X_S1_FENCED_CLOCK || X0X_S1_CRUSH_LFO_PROBE','X0X_S1_FENCED_CLOCK')
out[p]=s
p='firmware/src/dsp/s1c/s1c.c'
s=read(p)
s=one(s,'#if defined(X0X_S1_CRUSH_LFO_PROBE) && X0X_S1_CRUSH_LFO_PROBE\n#include <math.h>','#if defined(X0X_HOST) && defined(X0X_S1_CRUSH_LFO_PROBE) && X0X_S1_CRUSH_LFO_PROBE\n#include <math.h>')
s=one(s,'#ifndef X0X_HOST\n#error Synth One crush cadence witness is host only\n#endif\n','')
a='                float bsr = (float)log2(P(BCSR)) + 4.0f*lfo01(s,crush_route);\n                s1u_crush_clock(cr,clampf((float)exp2(bsr),2048.0f,48000.0f),s->sr);\n'
s=one(s,a,'#ifdef X0X_HOST\n'+a+'#else\n'+a.replace('(float)log2(', 'log2f(').replace('(float)exp2(', 'exp2f(')+'#endif\n')
out[p]=s
p=A+'sj6_137_host_witness.h'
s=read(p);s=clone(s,r'^#if X0X_S1_DELAY_TRACK_TEST\nenum \{ S1_DICT_BYTE=.*?^#endif\n'.replace('.*?',r'[\s\S]*?'))
s=one(s,'S1_CWG_BYTE=39, S1_CWG_MAX=50, S1_CWG_INIT=25, S1_CWG_PARAM=137','S1_CWG_BYTE=46, S1_CWG_MAX=127, S1_CWG_INIT=50, S1_CWG_PARAM=136')
s=s.replace('"S1 Delay tracking belongs in second owner row");\nstatic inline unsigned s1_cwg','"S1 Wet Makeup belongs in second owner row");\nstatic inline unsigned s1_cwg')
old='   if(i==S1_CRUSH_BYTE){if(x>S1_CRUSH_LAST)return 0;continue;}'
s=one(s,old,'''   if(i==S1_CRUSH_BYTE){
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
     if(x && !s1_typed_init_cell(s->v[T_303][v]))return 0;
#endif
     if(x>S1_CRUSH_LAST)return 0;
     continue;
   }''')
a='   if (x) return 0;'
s=one(s,a,'''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
   if(i==S1_CWG_BYTE){
     if(x>S1_CWG_MAX || (x && !s1_typed_init_cell(s->v[T_303][v])))return 0;
     continue;
   }
#endif
'''+a)
out[p]=s
p=A+'original_presets.inc'
s=read(p);s=clone(s,r'^#if X0X_S1_DELAY_TRACK_TEST\n/\* An Init-owned Delay input ratio,[\s\S]*?^#endif\n')
s=one(s,'/* An Init-owned Delay input ratio, never a T_MIX or global FX address. */\nstatic void s1_typed_cwg','/* Owner-local wet compressor makeup, not main gain or reverb input. */\nstatic void s1_typed_cwg')
s=one(s,'(float)(50u+s1_cwg_decode(code))*.01f','code ? .5f+(3.5f/127.f)*(float)s1_cwg_decode(code) : s1c_front_init(S1_CWG_PARAM)')
s=clone(s,r'^#if X0X_S1_DELAY_TRACK_TEST\n    if\(rc==ORIGINAL_RUNTIME_OK\)s1_typed_dict_apply\(sound,v\);\n#endif\n')
s=clone(s,r'^#if X0X_S1_DELAY_TRACK_TEST\n        s1_typed_dict_apply\(sound,v\);\n#endif\n')
out[p]=s
p=A+'ui.c'
s=read(p)
for pat in [
 r'^#if X0X_S1_DELAY_TRACK_TEST\nenum \{ R_S1DICT=[\s\S]*?^#endif\n',
 r'^#if X0X_S1_DELAY_TRACK_TEST\n    if \(r.kind==R_S1DICT\)[^\n]*\n#endif\n',
 r'^#if X0X_S1_DELAY_TRACK_TEST\n    case R_S1DICT:[^\n]*\n#endif\n',
 r'^#if X0X_S1_DELAY_TRACK_TEST\n    if\(r.kind==R_S1DICT\) \{ put_i[^\n]*\n#endif\n',
 r'^#if X0X_S1_DELAY_TRACK_TEST\n    unsigned undo_dict=[\s\S]*?^#endif\n',
 r'^#if X0X_S1_DELAY_TRACK_TEST\n                const uint32_t dict_off=[\s\S]*?^#endif\n',
 r'^#if defined\(DNBOX_137_HOST_WITNESS\) && X0X_S1_DELAY_TRACK_TEST\n    if\(undo_dict \&\& [\s\S]*?^#endif\n',
 r'^#if X0X_S1_DELAY_TRACK_TEST\n    if\(undo_dict\)[^\n]*\n#endif\n',
 r'^#if X0X_S1_DELAY_TRACK_TEST\n    if\(r.kind==R_S1DICT\) \{\n[\s\S]*?^#endif\n',
 ]: s=clone(s,pat)
s=clone(s,r'^#if X0X_S1_DELAY_TRACK_TEST\n    case R_S1DICT: \{int pos=[^\n]*\n#endif\n')
assert not re.search(r'R_NOTELOCK\s*\+\s*31\b',s)
s=one(s,'R_S1CWG=R_NOTELOCK+20','R_S1CWG=R_NOTELOCK+31')
s=one(s,'if(r.kind==R_S1CWG) { put_i(num,50+v);put_s(unit,"%");return; }',
      'if(r.kind==R_S1CWG) { put_i(num,v==S1_CWG_INIT ? 188 : 50+(350*v+63)/127);put_s(unit,"%");return; }')
s=s.replace('undo_feedback || undo_dict ||','undo_feedback || undo_cwg || undo_dict ||')
assert s.count('undo_feedback || undo_cwg || undo_dict ||')>=4
fx='                add_original_extra_pages(emit, s1_pages + effect_page, S1_EFFECT_PAGES, b, R_S1PATCH);'
s=one(s,fx,'''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
                if (live_s1_mod(&proj.sound,b)) {
                    add_page(emit,PT_EFFECT,PR(R_ENG,T_303,b,ORIGINAL_SYNTHONE_CRUSH),
                        PR(R_S1PATCH,T_303,b,S1_DELAY_MIX),PR(R_S1PATCH,T_303,b,S1_REVERB_MIX),
#if X0X_S1_CRUSH_LFO_PROBE
                        PR(R_S1CRUSH,T_303,b,0));
#else
                        NONE);
#endif
                    add_page(emit,PT_REVERB,PR(R_S1CWG,T_303,b,0),PR(R_S1PATCH,T_303,b,S1_REVERB_HP),
                        PR(R_S1PATCH,T_303,b,S1_DELAY_ON),PR(R_S1PATCH,T_303,b,S1_REVERB_ON));
                    add_original_extra_pages(emit,s1_pages+effect_page+2,S1_EFFECT_PAGES-2,b,R_S1PATCH);
                } else
#endif
'''+fx)
out[p]=s
for p,s in out.items(): (root/p).write_text(s)
