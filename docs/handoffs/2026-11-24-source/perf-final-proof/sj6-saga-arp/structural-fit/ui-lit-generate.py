"""Lab-only relocation of explicit normal-panel UI strings, not the merged pool."""
import re,sys
from pathlib import Path
p=Path(sys.argv[1]); s=p.read_text()
fn='pref_name draw_perf describe_pages run_action pref_set_impl button_tap sj6_knob_desc list_enter original_value_text draw_part_sound matrix_name do_undo hdr_name matrix_group reset_sound key_event draw_mix bass_display_name pref_value_of draw_303 arp_desc knob_tag drum_strip_role draw_home_live draw_header bass_header_name draw_knobs part_s input_body queued_save_message original_status_name undo_name queued_save_label add_track_pages s1_edit draw_fx lock_turn draw_main pref_desc bass_write_held_chord draw_drum_amp_inline draw_adsr_inline preset_shown motion_forget globe_paste_part ask_sound_reset prob_turn unit_value draw_param_widget draw_readout matrix_preset_name header_save_label save_project adsr_touch_active held_note_by bass_write bass_step_key set_view draw_drum_mix draw_home perf_test_abort'.split()
functions={f[1]:(f.end(),s.index('\n}',f.end())) for f in re.finditer(r'^(?:X0X_COLD )?static [^\n;{]+\b(\w+)\([^;{}]*?\)\n\{',s,re.M) if f[1] in fn}
assert set(fn)==functions.keys()
lex=r'/\*[\s\S]*?\*/|//[^\n]*|\'(?:\\.|[^\'\\])*\'|("(?:\\.|[^"\\])*")'
vals={}; repl=[]
for name,(a,b) in functions.items():
    t=s[a:b]
    for m in re.finditer(lex,t):
        literal=m[1]
        if not literal or len(literal)<6 or not literal[1:-1].isascii(): continue
        if '\\' in literal:continue
        # Avoid literal concatenation and preprocessor directives; keep all
        # spec declarations and other use sites unchanged.
        left=t[:m.start()].rstrip()
        right=t[m.end():].lstrip()
        if (left.endswith('"') or right.startswith('"') or
            re.search(r'\b(?:UI_CAP_\w+|X0X_\w+)\s*$',left) or re.match(r'(?:UI_CAP_\w+|X0X_\w+)\b',right)): continue
        if t[t.rfind('\n',0,m.start())+1:m.start()].lstrip().startswith('#'):continue
        if literal not in vals:vals[literal]=len(vals)
        repl.append((a+m.start(),a+m.end(),f'LU_{vals[literal]}'))
for a,b,k in sorted(repl,reverse=True):s=s[:a]+k+s[b:]
pos=s.index('#include ')
s=s[:pos]+'#include "ui_lit_companion_lab.h"\n'+s[pos:]
p.write_text(s)
h=['/* Exact strings from the normal panel UI. No safety or update path. */',
   '#pragma once','#if defined(X0X_COLD_UI_LIT_LAB)']
h += [f'static const char LU_{i}[] __attribute__((section(".lab_ui_copy.c{i}"))) = {x};' for x,i in vals.items()]
h += ['#else']+[f'#define LU_{i} {x}' for x,i in vals.items()]+['#endif']
p.with_name('ui_lit_companion_lab.h').write_text('\n'.join(h)+'\n')
print('foreground source Functions',len(functions),'literal occurrences',len(repl),'unique',len(vals),'upper source bytes',sum(len(x)-1 for x in vals))
