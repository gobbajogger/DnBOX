from pathlib import Path
import sys,difflib
base=Path(sys.argv[1])
out=Path(__file__).parent
diff=[]
for f in ('dsp/juno6.h','dsp/juno6_factory.inc','app/original_runtime.c'):
 p=base/'firmware/src'/f; old=p.read_text(); s=old
 def ins(a,b):
  global s
  assert s.count(a)==1,(f,a,s.count(a))
  s=s.replace(a,b)
 if f.endswith('.h'):
  ins(' uint8_t lfo1_cut_active, lfo1_cut_force;', ''' uint8_t lfo1_cut_active, lfo1_cut_force;
#if X0X_SJ6_NOTE_CUTOFF_LATCH
 float note_cut_current, note_cut_target;
 uint8_t note_cut_latched;
#endif''')
  ins('int juno6_configure_lfo1_cutoff(', '''#if X0X_SJ6_NOTE_CUTOFF_LATCH
int juno6_cutoff_note_qualifies(const juno6_t *);
void juno6_cutoff_first_note(juno6_t *);
#endif
int juno6_configure_lfo1_cutoff(''')
 elif f.endswith('.c'):
  ins('        ++*held;', '''#if X0X_SJ6_NOTE_CUTOFF_LATCH
        if (!*held && p->kind==ORIGINAL_SOUND_JUNO &&
            juno6_cutoff_note_qualifies(&p->dsp.juno.synth)) {
            unsigned any=0;
            for(unsigned s=0;s<ORIGINAL_INPUT_COUNT && !any;s++)
                for(unsigned k=0;k<128u;k++) if(p->keys[s][k]) {any=1;break;}
            if(!any)juno6_cutoff_first_note(&p->dsp.juno.synth);
        }
#endif
        ++*held;''')
 else:
  ins('int juno6_configure_lfo1_cutoff(', '''#if X0X_SJ6_NOTE_CUTOFF_LATCH
#include "juno6_note_cutoff.h"
#endif
int juno6_configure_lfo1_cutoff(''')
  ins(' s->macro[id]=x;', '''#if X0X_SJ6_NOTE_CUTOFF_LATCH
 if(id==2 && s->typed && s->note_cut_latched) {
  s->note_cut_target=sj6_note_cut_rebase(s->note_cut_target,s->macro[2],x);
  s->macro[2]=x;
  (void)dnbx_store_raw(&s->core,7u,x);
  return 1;
 }
#endif
 s->macro[id]=x;''')
  ins('        s->lfo1_cut_active=s->lfo1_cut_force=0;\n        s->lfo1_cut_until=', '''        s->lfo1_cut_active=s->lfo1_cut_force=0;
#if X0X_SJ6_NOTE_CUTOFF_LATCH
        s->note_cut_latched=0;
#endif
        s->lfo1_cut_until=''')
  ins('static int sj6_render_lfo1_cutoff(juno6_t *s,float *left,float *right,unsigned frames)', 'static int sj6_render_lfo1_cutoff(juno6_t *s,float *left,float *right,unsigned frames,unsigned run_lfo)')
  ins('            sj6_cutoff_tick(s,','''#if X0X_SJ6_NOTE_CUTOFF_LATCH
            if(s->note_cut_latched) {
                if(run_lfo && sj6_effective_lfo_amount(s,0u)>.01f) {
                    /* The active LFO may subsequently replace the note target. */
                    s->lfo1_cut_active=1;
                    s->lfo1_cut_current=s->note_cut_current;
                    s->note_cut_latched=0;
                } else {
                    s->mod_wheel_cutoff_hz=sj6_cutoff_hz(s,sj6_note_cut_tick(s));
                }
            }
            if(!s->note_cut_latched && run_lfo)
#else
            (void)run_lfo;
#endif
            sj6_cutoff_tick(s,''')
  ins(' const unsigned cut_route=lfo1_route && (routed_cut || (legacy_lfo1 && s->lfo1_pwm_route==3));',''' const unsigned cut_route=lfo1_route && (routed_cut || (legacy_lfo1 && s->lfo1_pwm_route==3));
#if X0X_SJ6_NOTE_CUTOFF_LATCH
 const unsigned note_cut=s->note_cut_latched && s->mod_destination!=5u;
#else
 const unsigned note_cut=0;
#endif''')
  assert s.count('if (wheel_cutoff||cut_route)')==2
  s=s.replace('if (wheel_cutoff||cut_route)','if (wheel_cutoff||cut_route||note_cut)')
  ins('   rendered=cut_route && frames<=DNBX_MAX_FRAMES && left && right && left!=right ?\n      sj6_render_lfo1_cutoff(s,left,right,frames):render_typed_span(s,left,right,frames);','''   rendered=(cut_route||note_cut) && frames<=DNBX_MAX_FRAMES && left && right && left!=right ?
      sj6_render_lfo1_cutoff(s,left,right,frames,cut_route):render_typed_span(s,left,right,frames);''')
 a='a/firmware/src/'+f; b='b/firmware/src/'+f
 diff+=difflib.unified_diff(old.splitlines(True),s.splitlines(True),a,b)
 target=out/'src'/f; target.parent.mkdir(parents=True,exist_ok=True); target.write_text(s)
f='firmware/src/dsp/juno6_note_cutoff.h'
diff+=difflib.unified_diff([], (out/'src/dsp/juno6_note_cutoff.h').read_text().splitlines(True),'/dev/null','b/'+f)
(out/'candidate.patch').write_text(''.join(diff))
