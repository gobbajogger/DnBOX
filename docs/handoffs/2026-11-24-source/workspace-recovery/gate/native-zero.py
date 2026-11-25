#!/usr/bin/env python3
"""Generate native zero Gate delta against the exact joined donor."""
from pathlib import Path
import difflib, hashlib, re, sys
here = Path(__file__).resolve().parent
roots = [Path(arg) for arg in sys.argv[1:] if arg != '--partial']
partial = '--partial' in sys.argv[1:]
if not roots:
    raise SystemExit('usage: native-zero.py [--partial] EXACT_ROOT [EXACT_ROOT ...]')
paths = ('app/typed_controls_trial.h','app/sj6_137_host_witness.h','app/engine_arp.inc','app/ui.c','app/original_runtime.c','dsp/synthone/sequencer/S1Sequencer.c','dsp/synthone/sequencer/S1Sequencer.h')
paths = tuple('firmware/src/'+p for p in paths)
manifest = dict(line.strip().split(maxsplit=1)[::-1] for line in (here/'FIRMWARE.sha256').read_text().splitlines())
files = {}
def sha(s): return hashlib.sha256(s).hexdigest()
for p in paths:
    source = next((root/p for root in roots if (root/p).exists()), None)
    if source is None:
        if partial: continue
        raise SystemExit(f'Missing donor preimage: {p}')
    b = source.read_bytes()
    if sha(b) != manifest[p]:
        raise SystemExit(f'Wrong donor preimage: {p}: got {sha(b)}, expected {manifest[p]}')
    files[p] = b.decode()
def rep(s, a, b):
    assert s.count(a) == 1, (a,s.count(a))
    return s.replace(a,b)
def extend(s):
    probe = 'defined(X0X_SJ6_ZERO_GATE_PROBE) && X0X_SJ6_ZERO_GATE_PROBE'
    def line(m):
        l = m.group(0)
        if f'({probe})' in l:
            return l.replace(f'({probe})',f'(({probe}) || X0X_SJ6_NATIVE_ZERO_GATE)')
        if probe in l:
            return l.replace(probe, f'({probe}) || X0X_SJ6_NATIVE_ZERO_GATE')
        return l.replace('X0X_SJ6_ZERO_GATE_PROBE','X0X_SJ6_ZERO_GATE_PROBE || X0X_SJ6_NATIVE_ZERO_GATE')
    return re.sub(r'^#(?:if|elif) .*X0X_SJ6_ZERO_GATE_PROBE.*$',line,s,flags=re.M)

def change_typed(typed):
    typed = rep(typed,'#if X0X_SJ6_NATIVE_POSITIVE_GATE &&', '''#if X0X_SJ6_NATIVE_ZERO_GATE && (!DNBOX_TYPED_CONTROLS_NATIVE_TEST || !X0X_SJ6_NATIVE_POSITIVE_GATE)
#error "Native SuperJ6 zero Gate requires the typed native positive Gate carrier"
#endif
#if X0X_SJ6_NATIVE_POSITIVE_GATE &&''')
    return typed

def change_witness(witness):
    witness = extend(witness)
    witness = rep(witness,'if(i==SJ6_ARP_ZERO_BYTE){if(x>1u)return 0;continue;}','''if(i==SJ6_ARP_ZERO_BYTE){
    if(x>1u)return 0;
#if X0X_SJ6_NATIVE_ZERO_GATE
    if(x && (s->v[T_303][v][29]!=ORIGINAL_SOUND_JUNO || original_sound_sj6_factory(s->v[T_303][v])))return 0;
#endif
    continue;
   }''')
    return witness

def change_arp(arp):
    arp = extend(arp)
    arp = rep(arp, '''    if (arp_kind_ok(v)
#if X0X_SJ6_NATIVE_RANDOM2''', '''    if (arp_kind_ok(v)
#if X0X_SJ6_NATIVE_ZERO_GATE
        && (c->gate || (original_bass[v].kind==ORIGINAL_SOUND_JUNO && live137(mot_base_snd,v)))
#endif
#if X0X_SJ6_NATIVE_RANDOM2''')
    gate_only = '''#if X0X_SJ6_NATIVE_ZERO_GATE
static int arp_same_except_gate(const engine_arp_cfg *a, const engine_arp_cfg *b)
{
    uint8_t x[ARP_BLOCK_BYTES], y[ARP_BLOCK_BYTES];
    if (a->mode!=b->mode) return 0;  /* RANDOM2 is not encoded in the legacy block. */
    engine_arp_block_encode(a,x);
    engine_arp_block_encode(b,y);
    x[5]=y[5];
    return !__builtin_memcmp(x,y,sizeof x);
}
#endif
'''
    arp = rep(arp, 'static int arp_queue(', gate_only+'static int arp_queue(')
    arp = rep(arp, '''            arp_gate_only[v]=0;
#endif
            arp_pending[v] = *c;''', '''#if X0X_SJ6_NATIVE_ZERO_GATE
            arp_gate_only[v]=(uint8_t)(c->gate!=original_bass[v].arp.cfg.gate &&
                arp_same_except_gate(c,&original_bass[v].arp.cfg));
#else
            arp_gate_only[v]=0;
#endif
#endif
            arp_pending[v] = *c;''')
    arp = rep(arp, '''        arp_pending_flag[v]=0;
        (void)original_runtime_arp_config(&original_bass[v],&c);''', '''        arp_pending_flag[v]=0;
#if X0X_SJ6_NATIVE_ZERO_GATE
        if (!(arp_same_except_gate(&c,&original_bass[v].arp.cfg) &&
              original_runtime_sj6_live_gate(&original_bass[v],c.gate)))
#endif
            (void)original_runtime_arp_config(&original_bass[v],&c);''')
    return arp

def change_ui(ui):
    ui = extend(ui)
    return ui

def change_rt(rt):
    rt = rep(rt, '#error "Saga zero Gate is a composed host witness only"\n#endif\n    if (p->kind==ORIGINAL_SOUND_JUNO) q->seqGate=c->gate;', '''#error "Saga zero Gate is a composed host witness only"
#endif
    if (p->kind==ORIGINAL_SOUND_JUNO) q->seqGate=c->gate;
#elif X0X_SJ6_NATIVE_ZERO_GATE
    if (p->kind==ORIGINAL_SOUND_JUNO && p->dsp.juno.synth.typed) q->seqGate=c->gate;''')
    rt = rep(rt, 'gate<1u || gate>100u) return 0;', 'gate>100u\n#if !X0X_SJ6_NATIVE_ZERO_GATE\n       || gate<1u\n#endif\n       ) return 0;')
    return rt

def change_seq(seq):
    seq = rep(seq, 'if (s->liveGateTicks) return s->liveGateTicks;', '''#if X0X_SJ6_NATIVE_ZERO_GATE
    if (s->liveGateLatched) return s->liveGateTicks;
#else
    if (s->liveGateTicks) return s->liveGateTicks;
#endif''')
    old='''    /* Positive legacy Gate has a nonzero threshold at the shortest legal step. */
    if (s->gateOpen && s->sequencerLastNotesCount && !s->liveGateTicks)
        s->liveGateTicks=p->gateTicks;'''
    seq = rep(seq,old,'''#if X0X_SJ6_NATIVE_ZERO_GATE
    if (s->gateOpen && s->sequencerLastNotesCount && !s->liveGateLatched) {
        s->liveGateTicks=p->gateTicks;
        s->liveGateLatched=1;
    }
#else
'''+old+'''
#endif''')
    seq = seq.replace('    s->liveGateTicks=0;','''    s->liveGateTicks=0;
#if X0X_SJ6_NATIVE_ZERO_GATE
    s->liveGateLatched=0;
#endif''')
    assert seq.count('s->liveGateLatched=0;')==2
    return seq

def change_header(header):
    header = rep(header,'uint32_t liveGateTicks;            /* prior active Saga gate; 0 unlatched */','''uint32_t liveGateTicks;            /* prior active Saga deadline */
#if X0X_SJ6_NATIVE_ZERO_GATE
    uint8_t liveGateLatched;          /* zero is itself a deadline */
#endif''')

    return header

changes = zip(paths, (change_typed,change_witness,change_arp,change_ui,change_rt,change_seq,change_header))
pre,post,patch=[],[],[]
for p,change in changes:
    if p not in files: continue
    new = change(files[p])
    assert new != files[p], p
    pre.append(f'{manifest[p]}  {p}\n')
    post.append(f'{sha(new.encode())}  {p}\n')
    patch.extend(difflib.unified_diff(files[p].splitlines(True),new.splitlines(True), 'a/'+p, 'b/'+p))
for name,content in [('native-zero.patch',patch),('native-zero.PRE.sha256',pre),('native-zero.POST.sha256',post)]:
    (here/(('partial-' if partial else '') + name)).write_text(''.join(content))
