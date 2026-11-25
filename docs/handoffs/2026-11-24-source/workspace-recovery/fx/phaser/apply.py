#!/usr/bin/env python3
"""Apply compiled Phaser carrier to the composed typed lab. Fail closed on missed anchors."""
from pathlib import Path
import re,sys
here=Path(__file__).parent
root=Path(sys.argv[1])/'firmware/src/app'
def splice(s,a,b):
    if s.count(a)!=1: raise ValueError((s.count(a),a))
    return s.replace(a,b,1)
def before(s,a,b): return splice(s,a,b+a)
def after(s,a,b): return splice(s,a,a+b)
def guarded(s): return '#if DNBOX_TYPED_CONTROLS_NATIVE_TEST\n'+s+'#endif\n'
p=root/'sj6_137_host_witness.h'; w=p.read_text()
w=before(w,'static inline int s1_typed_init_cell(const uint8_t *b);',guarded('#include "s1_native_phaser.h"\n'))
w=before(w,'   if (x) return 0;',guarded('''   if(i>=S1_PHASE_BYTE && i<S1_PHASE_BYTE+S1_PHASE_COUNT){
     if(x>S1_PHASE_MAX || (x && !s1_typed_init_cell(s->v[T_303][v])))return 0;
     continue;
   }
'''))
q=root/'original_presets.inc'; e=q.read_text()
e=before(e,'static void s1_typed_a4_apply(const sound_t *sound,unsigned owner)',(here/'engine.inc').read_text())
for a in ('    if(rc==ORIGINAL_RUNTIME_OK) {s1_typed_a4_apply(sound,v);','        s1_typed_a4_apply(sound,v);'):
    e=before(e,a,guarded(('    if(rc==ORIGINAL_RUNTIME_OK)' if a.startswith('    if') else '        ')+'s1_phaser_apply(sound,v);\n'))
r=root/'ui.c'; u=r.read_text()
m=re.search(r'^(?:enum \{ R_S1DICT|enum \{ R_S1MODWHEEL).*?;\n',u,re.M)
if not m: raise ValueError('typed UI absent or incomplete')
assert not re.search(r'R_NOTELOCK\s*\+\s*32\b',u)
u=after(u,m[0],guarded('''enum { R_S1PHASER=R_NOTELOCK+32 };
extern int s1_phaser_get(unsigned,unsigned);
extern int s1_phaser_choose(unsigned,unsigned,unsigned);
extern int s1_phaser_undo_ready(unsigned);
extern void s1_phaser_undo(unsigned);
'''))
u=after(u,'static int pref_read_only(pref_t r)\n{\n',guarded('    if(r.kind==R_S1PHASER)return s1_phaser_get(r.b,r.c)<0 || !s1_typed_tuple_editable();\n'))
u=before(u,'    case R_BSTATUS: return &BSTATUS_P;',guarded('''    case R_S1PHASER: {
        static const x0x_param_t p[4]={{"MIX",127,0,0},{"RATE",127,5,0},
            {"FEEDBACK",127,0,0},{"WIDTH",127,99,0}};
        return r.c<S1_PHASE_COUNT?&p[r.c]:0;
    }
'''))
u=before(u,'    case R_BSTATUS: return engine_bass_preset_pending',guarded('    case R_S1PHASER: {int v=s1_phaser_get(r.b,r.c);return v>=0 ? v : s1_phase_init(r.c);}\n'))
u=before(u,'    if (r.kind == R_S1PATCH) { s1_edit(r, v, 0); return; }',guarded('    if(r.kind==R_S1PHASER){if(!pref_read_only(r) && v>=0 && v<=S1_PHASE_MAX && s1_phaser_choose(r.b,r.c,(unsigned)v))mark_dirty();return;}\n'))
u=before(u,'    if (r.kind == R_S1PATCH) { s1_edit(r, accel(knob, e, S1_PATCH_STEPS), 1); return; }',guarded('''    if(r.kind==R_S1PHASER){
        int pos=pref_get(r)+(e>0)-(e<0);
        if(e && pos>=0 && pos<=S1_PHASE_MAX && !pref_read_only(r) && s1_phaser_choose(r.b,r.c,(unsigned)pos))mark_dirty();
        return;
    }
'''))
u=after(u,'static void pref_value_of(pref_t r, int v, char *num, char *unit)\n{\n',guarded('''    if(r.kind==R_S1PHASER){
        float x=s1_phase_value(r.c,s1_phase_encode(r.c,(unsigned)v));
        if(r.c==0u || r.c==2u) {put_i(num,(int)(x*100.f+.5f));put_s(unit,"%");}
        else {put_i(num,(int)(x+.5f));put_s(unit,r.c==1u?"/MIN":"Hz");}
        return;
    }
'''))
a='                if (live_s1_mod(&proj.sound,b)) add_page(emit,PT_TUNING,'
u=before(u,a,guarded('''                if (live_s1_mod(&proj.sound,b))
                    add_page(emit,PT_PHASER,PR(R_S1PHASER,T_303,b,0),PR(R_S1PHASER,T_303,b,1),
                        PR(R_S1PHASER,T_303,b,2),PR(R_S1PHASER,T_303,b,3));
'''))
# Use descriptor title IDs, not string ptrs, on capacity-composed UI.
if not re.search(r'\[PT_PHASER\]\s*=',u):
    u=before(u,'    PT_DELAY,','    PT_PHASER,\n')
    u=before(u,'    [PT_DELAY] =','    [PT_PHASER] = "PHASER",\n')
u=before(u,'    unsigned undo_velocity137=',guarded('    unsigned undo_phaser=0;\n')+'#if !DNBOX_TYPED_CONTROLS_NATIVE_TEST\n    const unsigned undo_phaser=0;\n#endif\n')
u=after(u,'                uint32_t off=cell+(8u+v)*NPARAMS_MAX+2u;\n',guarded('''                uint8_t ph[S1_PHASE_COUNT];
                undo_peek(&undo,entry,off-2u+S1_PHASE_BYTE,ph,sizeof ph);
                for(unsigned k=0;k<S1_PHASE_COUNT;k++)
                    if(ph[k]>S1_PHASE_MAX){say("UNDO PHASER INVALID",0);engine_bass_edit_hold(0);return;}
                if(memcmp(ph,s1_phase_read(&proj.sound,v),sizeof ph))undo_phaser|=1u<<v;
'''))
assert u.count('undo_feedback ||')>=4
u=u.replace('undo_feedback ||','undo_feedback || undo_phaser ||')
u=before(u,'    if(undo_crush) s1_typed_crush_undo(',guarded('    if(undo_phaser)s1_phaser_undo(undo_phaser & ~undo_bass_mask);\n'))
u=before(u,'    undo_lanes = 0;\n    if (!(redo ? undo_redo',guarded('''    if(undo_phaser && !s1_phaser_undo_ready(undo_phaser & ~undo_bass_mask)) {
        engine_project_hold(0);engine_bass_edit_hold(0);say("UNDO WAIT PHASER",0);return;
    }
'''))
assert not (root/'s1_native_phaser.h').exists()
for path,body in [(p,w),(q,e),(r,u),(root/'s1_native_phaser.h',(here/'s1_native_phaser.h').read_text())]: path.write_text(body)
