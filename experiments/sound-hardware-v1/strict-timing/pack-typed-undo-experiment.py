"""Strict donor-shaped experimental closure, preserving exact guarded Undo order."""
from pathlib import Path
import re,sys
p=Path(sys.argv[1]);s=p.read_text()
a=s.index('static void do_undo(int redo)');b=s.index('/* SAVE while playing',a);u=s[a:b]
names=list(dict.fromkeys(re.findall(r'ui_undo_apply\((undo_\w+),\w+\);',u)))
assert len(names)==27
def decl(m):
 h,ds=m.groups(); left=[x for x in ds.split(',') if x.strip().split('=')[0] not in names]
 return h+','.join(left)+';' if left else ''
u=re.sub(r'^([ \t]*(?:const )?unsigned )((?:undo_\w+=0)(?:,\s*undo_\w+=0)*);',decl,u,flags=re.M)
c,d=u.index('    extern void sj6_137_undo_apply(unsigned);'),u.index('    if(undo_bass_mask || typed_hold_changes) engine_project_hold(0);')
f=[];ids=[]
for l in u[c:d].splitlines():
 m=re.fullmatch(r'\s*ui_undo_apply\((undo_\w+),(\w+)\);',l)
 if m:f.append('        '+m[2]+',');ids.append('        '+str(names.index(m[1]))+',')
 elif l.startswith('#'):f.append(l);ids.append(l)
 else:assert 'extern ' in l,l
u=u[:c]+'''    extern void sj6_137_undo_apply(unsigned);
    extern void s1_typed_mod_undo(unsigned);
    static void (*const steps[])(unsigned)={
'''+ '\n'.join(f)+'''
    };
    static const unsigned char step_masks[]={
'''+ '\n'.join(ids)+'''
    };
    _Static_assert(sizeof steps/sizeof *steps==sizeof step_masks,"Undo apply table");
    for(unsigned k=0;k<sizeof steps/sizeof *steps;k++)
        ui_undo_apply(masks[step_masks[k]],steps[k]);
'''+u[d:]
c,d=u.index('#if X0X_SJ6_MONO32_PROBE\n    if(!ui_undo_check'),u.index('#if X0X_S1_DELAY_TIME_NATIVE\n    if(undo_dlytime');v=[]
for l in u[c:d].splitlines():
 m=re.fullmatch(r'\s*if\(!ui_undo_check\((undo_\w+),(\w+),\(\(const char \*\)\(fixed_ui_notices\+(\d+)\)\)\)\)return;',l)
 if m:
  i=names.index(m[1]);n=int(m[3]);assert i<32 and n<2048;v.append(f'        {{ {m[2]}, {n*32+i} }},')
 elif l.startswith('#'):v.append(l)
 else:assert not l.strip(),l
u=u[:c]+'''#if X0X_BASS_TAGGED_SOUND && defined(DNBOX_137_HOST_WITNESS)
    static const struct { int (*ready)(unsigned); unsigned short packed; } verifies[]={
'''+ '\n'.join(v)+'''
    };
    for(unsigned k=0;k<sizeof verifies/sizeof *verifies;k++) {
        unsigned data=verifies[k].packed;
        if(!ui_undo_check(masks[data&31u],verifies[k].ready,
                          (const char *)(fixed_ui_notices+(data>>5))))return;
    }
#endif
'''+u[d:]
for i,n in enumerate(names):u=re.sub(r'\b'+n+r'\b','masks['+str(i)+']',u)
anchor='    unsigned typed_hold_changes=0,undo_sj6_input_vel=0;'
assert u.count(anchor)==1
u=u.replace(anchor,f'    unsigned masks[{len(names)}]={{0}};\n'+anchor)
p.write_text(s[:a]+u+s[b:])
