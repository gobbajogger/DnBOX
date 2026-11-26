"""Generate quarantined factory fallback source delta against pinned H16 baseline."""
import difflib,sys
from pathlib import Path
d=Path(__file__).resolve().parent
t=d.parent/'s1-organ-wet-recovery/native-widen-bfp/tree'
out=d/'private/patched'
def replace(s,x,y):
 assert s.count(x)==1,(x,s.count(x));return s.replace(x,y)
def apply(n,expr):
 p=t/'firmware/src'/n
 s=p.read_text()
 z=expr(s)
 dst=out/'firmware/src'/n
 dst.parent.mkdir(parents=True,exist_ok=True);dst.write_text(z)
 return ''.join(difflib.unified_diff(s.splitlines(True),z.splitlines(True),'a/firmware/src/'+n,'b/firmware/src/'+n))
patch=[]
patch.append(apply('dsp/synthone/sequencer/S1Sequencer.h',lambda s:replace(s,
    '    uint8_t arpTotalSteps;              /* 66  1..16 */',
    '''    uint8_t arpTotalSteps;              /* 66  1..16 */
#if X0X_S1_FACTORY_ARP_INGRESS
    const float *s1FactoryPitch;        /* owning Synth One's t[]; NULL for user project and Saga */
#endif''')))
patch.append(apply('dsp/synthone/sequencer/S1Sequencer.c',lambda s:replace(replace(s,
    '                        nn=(int)((float)nn*npof); /* original truncates each transposed step */',
    '''#if X0X_S1_FACTORY_ARP_INGRESS
                        if (p->s1FactoryPitch)
                            nn=(int)(p->s1FactoryPitch[67u+i]*npof);
                        else
#endif
                        nn=(int)((float)nn*npof); /* original truncates each transposed step */'''),
    '                                          (int)((float)p->arpInterval*npof),',
    '''#if X0X_S1_FACTORY_ARP_INGRESS
                                          (int)((p->s1FactoryPitch ? p->s1FactoryPitch[61] :
                                                       (float)p->arpInterval)*npof),
#else
                                          (int)((float)p->arpInterval*npof),
#endif''')))
patch.append(apply('app/original_runtime.h',lambda s:replace(s,'int original_runtime_s1_clock(original_runtime_part *, float, unsigned);','''int original_runtime_s1_clock(original_runtime_part *, float, unsigned);
#if X0X_S1_FACTORY_ARP_INGRESS
int original_runtime_s1_factory_cfg(const original_runtime_part *, engine_arp_cfg *, int saved, int pending);
int original_runtime_s1_factory_clock(original_runtime_part *);
#endif''')))
patch.append(apply('app/original_runtime.c',lambda s:replace(replace(replace(s,
    '    case 61: c->interval = (uint8_t)arp_round(v, 0, 12); return 1;',
    '''    case 61:
#if X0X_S1_FACTORY_ARP_INGRESS
        /* Factory UI mirror only; the sequencer keeps the authored float
         * until after notes-per-octave scaling. Manual project values unchanged. */
        c->interval = v < 0.f ? 0u : v > 12.f ? 12u : (uint8_t)v;
#else
        c->interval = (uint8_t)arp_round(v, 0, 12);
#endif
        return 1;'''),
    'static S1ClockPulse source_clock_sample(original_runtime_arp *a)',
    '''#if X0X_S1_FACTORY_ARP_INGRESS
#if !X0X_S1C || !X0X_S1_CLOCK_NATIVE || !X0X_S1_SCALE_NATIVE || !X0X_SJ6_ARP_ADAPTER_NATIVE || X0X_SYNTH_CONTROLS_INIT_ONLY
#error Factory fallback requires real Synth One pack, native source clock, scale and separate ARP/SEQ receiver
#endif
static int s1_factory_stop(const float *v)
{
    union { float f; uint32_t u; } value = {.f=v[147]};
    for (unsigned i=0;i<19u;++i)
        if (source_factor_bits(i)==value.u) return (int)i;
    return -1;
}
int original_runtime_s1_factory_cfg(const original_runtime_part *p, engine_arp_cfg *next, int saved, int pending)
{
    if (saved || pending || !p || !next || p->kind!=ORIGINAL_SOUND_SYNTHONE || p->status!=ORIGINAL_RUNTIME_OK ||
        p->dsp.synthone.program==S1_PART_INIT ||
        p->dsp.synthone.program>=s1_part_programs()) return 0;
    const float *v=p->dsp.synthone.e.t;
    const float bpm=v[64];
    if (v[62]!=1.f || !(bpm>=1.f&&bpm<=200.f) || bpm!=(float)(unsigned)bpm ||
        s1_factory_stop(v)<0 || !(v[61]>=0.f&&v[61]<=12.f)) return 0;
    for (unsigned i=67;i<=82;++i) if (!(v[i]>=-12.f&&v[i]<=12.f)) return 0;
    engine_arp_cfg cfg;
    engine_arp_cfg_default(&cfg);
    for (unsigned i=60;i<=114;++i) if (!engine_arp_from_s1(&cfg,i,v[i])) return 0;
    *next=cfg;
    return 1;
}
int original_runtime_s1_factory_clock(original_runtime_part *p)
{
    const float *v=p->arp.prm.s1FactoryPitch;
    if (!v) return 0;
    const int i=s1_factory_stop(v);
    return i>=0 && original_runtime_s1_clock(p,v[64],(unsigned)i);
}
#endif
static S1ClockPulse source_clock_sample(original_runtime_arp *a)'''),
    '    a->cfg = *c;\n    if (!ok) return 0;',
    '''    a->cfg = *c;
#if X0X_S1_FACTORY_ARP_INGRESS
    a->prm.s1FactoryPitch = NULL; /* Project/edit re-admission supersedes the factory fallback. */
#endif
    if (!ok) return 0;''')))
patch.append(apply('app/engine_arp.inc',lambda s:replace(replace(replace(replace(replace(s,
    '    (void)engine_arp_block_decode(sound ? ARP_SLOT(sound, v) : NULL, &c);',
    '''#if X0X_S1_FACTORY_ARP_INGRESS
    const int saved=engine_arp_block_decode(sound ? ARP_SLOT(sound, v) : NULL, &c);
    const int factory=sound && original_runtime_s1_factory_cfg(&original_bass[v], &c,saved,arp_pending_flag[v]);
#else
    (void)engine_arp_block_decode(sound ? ARP_SLOT(sound, v) : NULL, &c);
#endif'''),
    '    arp_adapt(&c,sound,(unsigned)v);',
    '''#if X0X_S1_FACTORY_ARP_INGRESS
    if (!factory)
#endif
        arp_adapt(&c,sound,(unsigned)v);'''),
    '        engine_arp_cfg c = arp_pending[v];',
    '''#if X0X_S1_FACTORY_ARP_INGRESS
        const int was_factory=original_bass[v].arp.prm.s1FactoryPitch!=NULL;
#endif
        engine_arp_cfg c = arp_pending[v];'''),
    '        arp_store(v, &c);\n        ++arp_revision[v];',
    '''        arp_store(v, &c);
#if X0X_S1_FACTORY_ARP_INGRESS
        if (was_factory && !original_bass[v].arp.prm.s1FactoryPitch)
            s1_typed_clock_apply(mot_base_snd,v);
#endif
        ++arp_revision[v];'''),
    '    arp_pending_flag[v] = 0;\n    (void)original_runtime_arp_config(&original_bass[v], &c);',
    '''    arp_pending_flag[v] = 0;
    const int admitted=original_runtime_arp_config(&original_bass[v], &c);
#if X0X_S1_FACTORY_ARP_INGRESS
    if (factory && admitted) original_bass[v].arp.prm.s1FactoryPitch=original_bass[v].dsp.synthone.e.t;
#else
    (void)admitted;
#endif''')))
patch.append(apply('app/original_presets.inc',lambda s:replace(s,
    '        (void)original_runtime_s1_clock(part,s1_clock_bpm(p),p[2]-1u);\n    else {',
    '''        (void)original_runtime_s1_clock(part,s1_clock_bpm(p),p[2]-1u);
#if X0X_S1_FACTORY_ARP_INGRESS
    else if (original_runtime_s1_factory_clock(part)) return; /* transient; no project writes */
#endif
    else {''')))
(d/'factory-arp-ingress.patch').write_text(''.join(patch))
