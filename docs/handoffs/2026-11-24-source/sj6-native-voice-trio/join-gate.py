"""Add already qualified positive-only Gate as a disjoint mixed-tree native trial path."""
from pathlib import Path
app=Path('firmware/src/app')
seq=Path('firmware/src/dsp/synthone/sequencer')
host='#if defined(X0X_SJ6_GATE_DEADLINE_PROBE) && X0X_SJ6_GATE_DEADLINE_PROBE'
native='#if X0X_SJ6_NATIVE_POSITIVE_GATE'
def load(p): return p.read_text()
def one(s,b,a):
    assert s.count(b)==1, b
    return s.replace(b,a)
def repl(p,b,a):
    p.write_text(one(load(p),b,a))
def alt(s,b,h):
    return one(s,host+'\n'+b+'\n#endif',host+'\n'+b+'\n#elif X0X_SJ6_NATIVE_POSITIVE_GATE\n'+h+'\n#endif')
p=app/'typed_controls_trial.h'
repl(p,'#if DNBOX_TYPED_CONTROLS_NATIVE_TEST\n#if !X0X_SYNTHONE_PATCH',
     '#if X0X_SJ6_NATIVE_POSITIVE_GATE && (!DNBOX_TYPED_CONTROLS_NATIVE_TEST || X0X_SJ6_ZERO_GATE_PROBE || X0X_SJ6_GATE_DEADLINE_PROBE)\n#error Positive Gate is a disjoint native trial, with zero and host Gate flags off\n#endif\n#if DNBOX_TYPED_CONTROLS_NATIVE_TEST\n#if !X0X_SYNTHONE_PATCH')
p=app/'engine_arp.inc'
s=load(p)
b='static uint8_t arp_gate_only[NBASS];'
s=one(s,host+'\n'+b,'#if X0X_SJ6_GATE_DEADLINE_PROBE || X0X_SJ6_NATIVE_POSITIVE_GATE\n'+b)
b='''        if (!(arp_gate_only[v] && live137(mot_base_snd,v) &&
              original_runtime_sj6_gate_edit(&original_bass[v],c.gate)))'''
s=alt(s,b,'''        if (!(arp_gate_only[v] && original_runtime_sj6_live_gate(&original_bass[v],c.gate)))''')
for b in ('            const int gate_field=', '            arp_gate_only[v]=0;'):
    s=one(s,host+'\n'+b,'#if X0X_SJ6_GATE_DEADLINE_PROBE || X0X_SJ6_NATIVE_POSITIVE_GATE\n'+b)
s=one(s,'#if X0X_SUPERJ6_BUILTIN_INIT\n        && (step != 0xFFu','#if X0X_SJ6_NATIVE_POSITIVE_GATE\n        && !(step==0xFFu && field==ARP_F_GATE && original_bass[v].kind==ORIGINAL_SOUND_JUNO &&\n             live137(mot_base_snd,v) && !s1_typed_tuple_editable())\n#endif\n#if X0X_SUPERJ6_BUILTIN_INIT\n        && (step != 0xFFu')
p.write_text(s)
p=app/'original_runtime.c'
s=load(p)
b='int original_runtime_arp_config(original_runtime_part *p, const engine_arp_cfg *c)'
p.write_text(one(s,b,native+'''
int original_runtime_sj6_live_gate(original_runtime_part *p,unsigned gate)
{
    if(!p || p->status!=ORIGINAL_RUNTIME_OK || p->kind!=ORIGINAL_SOUND_JUNO ||
       !p->dsp.juno.synth.typed || gate<1u || gate>100u) return 0;
    p->arp.cfg.gate=(uint8_t)gate;
    S1Sequencer_liveGate(&p->arp.seq,&p->arp.prm,gate);
    return 1;
}
#endif
'''+b))
p=app/'original_runtime.h'
s=load(p)
b='int original_runtime_arp_config(original_runtime_part *, const engine_arp_cfg *);'
p.write_text(one(s,b,native+'\nint original_runtime_sj6_live_gate(original_runtime_part *,unsigned);\n#endif\n'+b))
p=seq/'S1Sequencer.h'
s=load(p)
s=alt(s,"    uint32_t editGateTicks;           /* already dispatched step's original deadline */\n    uint8_t editGateLatched;","    uint32_t liveGateTicks;            /* prior active Saga gate; 0 unlatched */")
b=host+'\nvoid S1Sequencer_liveGate('
s=one(s,b,'#if X0X_SJ6_GATE_DEADLINE_PROBE || X0X_SJ6_NATIVE_POSITIVE_GATE\nvoid S1Sequencer_liveGate(')
p.write_text(s)
p=seq/'S1Sequencer.c'
s=load(p)
b=host+'\n    s->editGateLatched = 0;\n#endif'
a=host+'\n    s->editGateLatched = 0;\n#elif X0X_SJ6_NATIVE_POSITIVE_GATE\n    s->liveGateTicks=0;\n#endif'
assert s.count(b)==2
s=s.replace(b,a)
s=alt(s,'    if (s->editGateLatched) return s->editGateTicks;',
        '    if (s->liveGateTicks) return s->liveGateTicks;')
b=host+'\nvoid S1Sequencer_liveGate('
s=one(s,b,'#if X0X_SJ6_GATE_DEADLINE_PROBE || X0X_SJ6_NATIVE_POSITIVE_GATE\nvoid S1Sequencer_liveGate(')
b="""    if (s->gateOpen && s->sequencerLastNotesCount && !s->editGateLatched) {
        s->editGateTicks=p->gateTicks;
        s->editGateLatched=1;
    }"""
s=one(s,b,host+'\n'+b+'''
#else
    /* Positive legacy Gate has a nonzero threshold at the shortest legal step. */
    if (s->gateOpen && s->sequencerLastNotesCount && !s->liveGateTicks)
        s->liveGateTicks=p->gateTicks;
#endif''')
p.write_text(s)
