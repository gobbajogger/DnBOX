from pathlib import Path
import hashlib,sys
d,t=map(Path,sys.argv[1:]); t.mkdir(exist_ok=True)
flag='defined(X0X_HOST) && defined(SJ6_RATE_CARRIER_WITNESS)'
def edit(s,a,b):
 assert s.count(a)==1,a
 return s.replace(a,b)
for n in ('sj6_137_host_witness.h','original_presets.inc','ui.c','remote_sj6_panel.inc'):
 f=d/n;s=f.read_text();print(hashlib.sha256(f.read_bytes()).hexdigest(),n)
 if n=='sj6_137_host_witness.h':
  p='enum { SJ6_CRUSH_ROW=3, SJ6_CRUSH_BYTE=24, SJ6_CRUSH_ID=107, SJ6_CRUSH_AMOUNT_ID=108 };'
  s=edit(s,p,p+f'''
#if {flag}
enum {{ SJ6_CRUSH_RATE_FIELD=31, SJ6_CRUSH_RATE_ID=109 }};
#endif''')
  q='    [SJ6_MOD_BYTE]=SJ6_MOD_LAST,\n#endif'
  s=edit(s,q,q+f'''
#if {flag}
    [SJ6_CRUSH_RATE_FIELD]=128,
#endif''')
  q='    OWNER_BIT(SJ6_KEY_TRACK_BYTE)|OWNER_BIT(SJ6_ENV_STRENGTH_BYTE)|OWNER_BIT(SJ6_VCF_BYTE)|'
  s=edit(s,q,q+f'''
#endif
#if {flag}
    OWNER_BIT(SJ6_CRUSH_RATE_FIELD)|''')
 if n=='original_presets.inc':
  q='    if(k>1u || !original_typed_sj6_live(mot_base_snd,owner))'
  s=edit(s,q,f'''#if {flag}
    if(k==2u){{
        if(!original_typed_sj6_live(mot_base_snd,owner))return -1;
        unsigned code=read137(mot_base_snd,owner)[SJ6_CRUSH_RATE_FIELD];
        return code>128u?-1:code?(int)code-1:0; /* unassigned display, no applied receipt */
    }}
#endif
'''+q)
  q='    sj6_crush_write((sound_t*)mot_base_snd,owner)[k]=(uint8_t)pos;'
  s=edit(s,q,f'''#if {flag}
    if(k==2u){{
        slot137((sound_t *)mot_base_snd,owner)[SJ6_CRUSH_RATE_FIELD]=(uint8_t)(pos+1u);
        return; /* storage witness, never touches original_crush or graph */
    }}
#endif
'''+q)
  s=edit(s,'    if(pos>(k?127u:1u))return 0;','    if(pos>(k?127u:1u))return 0;')
  q='        memset(sj6_crush_write(sound,v),0,2u);'
  s=edit(s,q,q+f'''
#if {flag}
        slot137(sound,v)[SJ6_CRUSH_RATE_FIELD]=0;
#endif''')
 if n=='ui.c':
  q='        static const x0x_param_t p[2]={{"CRUSH",1,0,ONOFF_N},{"AMOUNT",127,0,0}};\n        return r.c<2u ? &p[r.c] : 0;'
  s=edit(s,q,f'''#if {flag}
        static const x0x_param_t p[3]={{{{"CRUSH",1,0,ONOFF_N}},{{"AMOUNT",127,0,0}},{{"BIT RATE",127,0,0}}}};
        return r.c<3u ? &p[r.c] : 0;
#else
'''+q+'\n#endif')
  q='                OWNER_PAGE(PT_EFFECT,J(R_SJ6CRUSH,0),J(R_SJ6CRUSH,1),0,0);'
  s=edit(s,q,f'''#if {flag}
                OWNER_PAGE(PT_EFFECT,J(R_SJ6CRUSH,0),J(R_SJ6CRUSH,1),J(R_SJ6CRUSH,2),0);
#else
'''+q+'\n#endif')
  q='                if(memcmp(crush,sj6_crush_code(&proj.sound,v),sizeof crush))undo_room|=1u<<v;'
  s=edit(s,q,q+f'''
#if {flag}
                const uint32_t ro=__builtin_offsetof(sound_t,v)+
                    ((T_303*NVOICES_MAX+8u+2u*v)*NPARAMS_MAX+SJ6_CRUSH_RATE_FIELD);
                uint8_t rate;
                undo_peek(&undo,entry,ro,&rate,1u);
                if(rate>128u){{undo_field_invalid("BIT RATE");return;}}
                /* Actual byte restore occurs in the existing Undo copy.
                 * No live effect means no stock room/Crusher callback. */
#endif''')
 if n=='remote_sj6_panel.inc':
  q='        {108, R_SJ6CRUSH, 1},'
  s=edit(s,q,q+f'''
#if {flag}
        {{109, R_SJ6CRUSH, 2}},
#endif''')
 (t/n).write_text(s)
