#!/usr/bin/env python3
"""Build-first b47 VCF port on the exact guarded Delay/Velocity donor."""
from pathlib import Path
import re, sys
root = Path(sys.argv[1]) / "firmware/src"
src = {}
def edit(file, old, new, n=1):
    if file not in src:
        src[file] = (root / file).read_text()
    s = src[file]
    actual = s.count(old)
    if actual != n:
        raise SystemExit(f"{file}: expected {n} occurrences, got {actual}: {old[:110]!r}")
    src[file] = s.replace(old, new)
h='dsp/juno6.h'
f='dsp/juno6_factory.inc'
t='app/sj6_137_host_witness.h'
p='app/original_presets.inc'
u='app/ui.c'
flag='X0X_SJ6_VCF_PORT'
def guard(body):
    return f'#if {flag}\n{body}\n#endif\n'
edit(h,'SJ6_FILTER_VELOCITY, SJ6_KNOBS', 'SJ6_FILTER_VELOCITY,\n#if X0X_SJ6_VCF_PORT\n       SJ6_VCF_LFO1_AMOUNT,\n#endif\n       SJ6_KNOBS')
edit(h,'int juno6_set_filter_velocity(juno6_t *, unsigned position);',
     'int juno6_set_filter_velocity(juno6_t *, unsigned position);\n'+guard('int juno6_set_vcf_lfo1_amount(juno6_t *, unsigned position);').rstrip())
edit(f,'int juno6_configure_lfo1_pwm(',guard('''/* Authored Init selector ID181=1 (normal cutoff) and ID180=0 are
 * constructor state. Neither is a panel knob or a wheel destination. */
int juno6_set_vcf_lfo1_amount(juno6_t *s,unsigned position)
{
    if (!s || !s->typed || position>127u) return 0;
    const float amount=(float)position/127.f;
    if(s->lfo1_pwm_route!=3u)
        return juno6_configure_lfo1_cutoff(s,0.f,1.f,amount);
    return dnbx_store_raw(&s->core,89u,amount)==DNBX_RAW_STORED_ONLY;
}''')+'int juno6_configure_lfo1_pwm(')
edit(t,'static inline int canonical137(const sound_t *s) {',guard('''enum { SJ6_VCF_BYTE=47 };
_Static_assert(SJ6_VCF_BYTE>=NPARAMS_MAX && SJ6_VCF_BYTE<2u*NPARAMS_MAX,
               "VCF amount belongs in the existing second owner row");
static inline const uint8_t *sj6_vcf_code(const sound_t *s,unsigned v)
{
    return &s->v[T_303][8+2u*v+SJ6_VCF_BYTE/NPARAMS_MAX][SJ6_VCF_BYTE%NPARAMS_MAX];
}
static inline int sj6_vcf_init(const sound_t *s,unsigned v)
{
    return s->v[T_303][v][29]==ORIGINAL_SOUND_JUNO &&
           !original_sound_sj6_factory(s->v[T_303][v]);
}''')+'static inline int canonical137(const sound_t *s) {')
edit(t,'   if (x) return 0;',guard('''   if(i==SJ6_VCF_BYTE) {
     if(x>127u || (x && !sj6_vcf_init(s,v)))return 0;
     continue;
   }''')+'   if (x) return 0;')
edit(p,'void sj6_137_undo_apply(unsigned mask)',guard('''static int sj6_vcf_apply(const sound_t *s,unsigned v)
{
    return live137(s,v) && sj6_vcf_init(s,v) &&
        original_bass[v].kind==ORIGINAL_SOUND_JUNO &&
        juno6_set_vcf_lfo1_amount(&original_bass[v].dsp.juno.synth,*sj6_vcf_code(s,v));
}
static void sj6_vcf_drop_pending(unsigned v)
{
    sj6_knob_mask[v].word[1]&=~sj6_knob_bit(SJ6_VCF_LFO1_AMOUNT);
}
int sj6_vcf_undo_ready(unsigned mask)
{
    for(unsigned v=0;v<NBASS;++v)
        if((mask&(1u<<v)) && (!live137(mot_base_snd,v) ||
            !sj6_vcf_init(mot_base_snd,v) || note_lock_applied[v] ||
            engine_bass_preset_pending(v) || original_bass[v].kind!=ORIGINAL_SOUND_JUNO))return 0;
    return 1;
}
void sj6_vcf_undo(unsigned mask)
{
    for(unsigned v=0;v<NBASS;++v)
        if(mask&(1u<<v)){sj6_vcf_drop_pending(v);(void)sj6_vcf_apply(mot_base_snd,v);}
}''')+'void sj6_137_undo_apply(unsigned mask)')
edit(p,'    if (slot == SJ6_OSC2_LEVEL || slot == SJ6_OSC1_LEVEL) {',
     guard('''    if(slot==SJ6_VCF_LFO1_AMOUNT)
        return live137(mot_base_snd,owner) && sj6_vcf_init(mot_base_snd,owner) ?
            *sj6_vcf_code(mot_base_snd,owner) : -1;''')+'    if (slot == SJ6_OSC2_LEVEL || slot == SJ6_OSC1_LEVEL) {')
edit(p,'        value = sj6_knob_target(before, value, (int)original_sound_sj6_max(slot), relative);',
     guard('''        if(slot==SJ6_VCF_LFO1_AMOUNT &&
           (before<0 || note_lock_applied[owner] || !s1_typed_tuple_editable())) {
            plat_cq_unlock(lock);return 0;
        }''')+'        value = sj6_knob_target(before, value, (int)original_sound_sj6_max(slot), relative);')
edit(p,'        unsigned upper = pending.word[1]',guard('''        if(pending.word[1] & sj6_knob_bit(SJ6_VCF_LFO1_AMOUNT)) {
            unsigned pos=sj6_knob_value[v][SJ6_VCF_LFO1_AMOUNT];
            if(!bass_named_preset[v] && live137(mot_base_snd,v) && sj6_vcf_init(mot_base_snd,v) &&
               s1_typed_tuple_editable() &&
               juno6_set_vcf_lfo1_amount(&original_bass[v].dsp.juno.synth,pos))
                *(uint8_t *)sj6_vcf_code(mot_base_snd,v)=(uint8_t)pos;
        }''')+'        unsigned upper = pending.word[1]')
reload='        if (live137(sound,v)) juno6_set_filter_velocity(&original_bass[v].dsp.juno.synth,read137(sound,v)[2]);'
edit(p,reload,reload+'\n'+guard('        (void)sj6_vcf_apply(sound,v);').rstrip(),n=2)
edit(u,'                    {PT_FILTER_MOD, {SJ6_FILTER_VELOCITY, ORIG_PAGE_NONE, ORIG_PAGE_NONE, ORIG_PAGE_NONE}},',
     '''#if X0X_SJ6_VCF_PORT
                    {PT_FILTER_MOD, {SJ6_FILTER_VELOCITY, SJ6_VCF_LFO1_AMOUNT, ORIG_PAGE_NONE, ORIG_PAGE_NONE}},
#else
                    {PT_FILTER_MOD, {SJ6_FILTER_VELOCITY, ORIG_PAGE_NONE, ORIG_PAGE_NONE, ORIG_PAGE_NONE}},
#endif''')
edit(u,'    unsigned undo_velocity137=0,',guard('''    extern int sj6_vcf_undo_ready(unsigned);
    extern void sj6_vcf_undo(unsigned);
    unsigned undo_vcf=0;
#else
    const unsigned undo_vcf=0;''')+'    unsigned undo_velocity137=0,')
edit(u,'                const uint32_t dict_off=off-2u+S1_DICT_BYTE;',
     '''#if X0X_SJ6_VCF_PORT
                const uint32_t vcf_off=__builtin_offsetof(sound_t,v)+
                    ((T_303*NVOICES_MAX+8u+2u*v)*NPARAMS_MAX+SJ6_VCF_BYTE);
                undo_peek(&undo,entry,vcf_off,&p,1u);
                if(p>127u){say("UNDO LFO1 INVALID",0);engine_bass_edit_hold(0);return;}
                if(p!=*sj6_vcf_code(&proj.sound,v))undo_vcf|=1u<<v;
#endif
                const uint32_t dict_off=off-2u+S1_DICT_BYTE;''')
# One preflight, two rollback/end releases, and one initial audio hold.
edit(u,' || undo_dict || undo_chorus_i', ' || undo_dict || undo_vcf || undo_chorus_i',n=4)
edit(u,'    if(undo_dict)s1_typed_dict_undo(undo_dict&~undo_bass_mask);',
     '    if(undo_dict)s1_typed_dict_undo(undo_dict&~undo_bass_mask);\n'+guard('    if(undo_vcf)sj6_vcf_undo(undo_vcf&~undo_bass_mask);').rstrip())
edit(u,'    if(undo_dict && !s1_typed_dict_undo_ready',
     guard('''    if(undo_vcf && !sj6_vcf_undo_ready(undo_vcf & ~undo_bass_mask)) {
        engine_project_hold(0);engine_bass_edit_hold(0);say("UNDO WAIT LFO1",0);return;
    }''')+'    if(undo_dict && !s1_typed_dict_undo_ready')
# Generic SuperJ6 parameter limits return 127; descriptor still must be explicit.
edit(u,'        {"VELOCITY", 127, 0, 0},\n        {"PWM BASE",',
     '        {"VELOCITY", 127, 0, 0},\n'+guard('        {"LFO1 AMOUNT", 127, 0, 0},')+'        {"PWM BASE",')
for file,s in src.items():
    (root/file).write_text(s)
