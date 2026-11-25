from pathlib import Path
from sys import argv
root=Path(argv[1])/'firmware/src'
def e(name,pairs):
 p=root/name;s=p.read_text()
 for a,b in pairs:
  assert s.count(a)==1,(name,a)
  s=s.replace(a,b)
 t=p.with_suffix('.new');t.write_text(s);t.replace(p)
s='dsp/synthone/sequencer/S1Sequencer'
e(s+'.h',[('#include <stdint.h>','#include <stdint.h>\n#if X0X_S1_FENCED_CLOCK && !defined(X0X_HOST)\n#error "Original clock prototype is host-only"\n#endif'),('int S1Sequencer_getArpBeatCount','''#if X0X_S1_FENCED_CLOCK
typedef struct { int reset, trigger, index; } S1ClockPulse;
void S1Sequencer_processOriginal(S1Sequencer *, const S1SequencerParams *,
    const NoteNumber *, int, void *, KeyOnCallback, KeyOffCallback, const S1ClockPulse *);
#endif
int S1Sequencer_getArpBeatCount''')])
e(s+'.c',[('void S1Sequencer_process(S1Sequencer *s,','''#if X0X_S1_FENCED_CLOCK
void S1Sequencer_processOriginal(S1Sequencer *s,
#else
void S1Sequencer_process(S1Sequencer *s,
#endif
'''),('void *ctx, KeyOnCallback on, KeyOffCallback off)\n{','''void *ctx, KeyOnCallback on, KeyOffCallback off
#if X0X_S1_FENCED_CLOCK
                         , const S1ClockPulse *pulse
#endif
                         )
{'''),('        if (newBeatTime >= p->arpSeqTempoMultiplier) {','''#if X0X_S1_FENCED_CLOCK
        if (pulse) {
            crossed = pulse->trigger;
            if (crossed) s->mStepCounter = pulse->index;
        } else
#endif
        if (newBeatTime >= p->arpSeqTempoMultiplier) {'''),('        s->mBeatTime = newBeatTime;','''#if X0X_S1_FENCED_CLOCK
        if (!pulse)
#endif
        s->mBeatTime = newBeatTime;'''),('        if (crossed || (firstTimeAnyKeysHeld','''        if (crossed || (
#if X0X_S1_FENCED_CLOCK
                        !pulse &&
#endif
                        firstTimeAnyKeysHeld'''),('                                           : s->mStepCounter - s->mStepBase) % size;','''                                           : s->mStepCounter - s->mStepBase) % size;
#if X0X_S1_FENCED_CLOCK
                    if (pulse) seqNotePosition = pulse->index % size;
#endif'''),('int S1Sequencer_getArpBeatCount','''#if X0X_S1_FENCED_CLOCK
void S1Sequencer_process(S1Sequencer *s, const S1SequencerParams *p, const NoteNumber *held, int count,
                         void *ctx, KeyOnCallback on, KeyOffCallback off)
{
    S1Sequencer_processOriginal(s,p,held,count,ctx,on,off,0);
}
#endif
int S1Sequencer_getArpBeatCount''')])
e('app/original_runtime.h',[('    float bpm;','''    float bpm;
#if X0X_S1_FENCED_CLOCK
    double source_phase, source_delta;
    float source_factor;
    uint8_t source_set, source_wait;
#endif'''),('int original_runtime_arp_view','''#if X0X_S1_FENCED_CLOCK
int original_runtime_s1_clock(original_runtime_part *, float, unsigned);
int engine_host_s1_clock(unsigned, float, unsigned);
#endif
int original_runtime_arp_view''')])
e('app/original_runtime.c',[('void original_runtime_arp_clock','''#if X0X_S1_FENCED_CLOCK
int original_runtime_s1_clock(original_runtime_part *p, float bpm, unsigned factor)
{
    static const uint16_t units[]={1536,1152,768,576,384,192,128,96,64,48,32,24,16,12,8,6,4,3,2};
    if (!p || p->kind!=ORIGINAL_SOUND_SYNTHONE || p->status!=ORIGINAL_RUNTIME_OK ||
        !(bpm>=1.f && bpm<=200.f) || factor>=sizeof units/sizeof units[0]) return 0;
    original_runtime_arp *a=&p->arp;
    if (!a->source_set) a->source_wait=!!a->nheld;
    a->source_set=1;
    a->source_delta=(bpm/60.f)/44100.;
    a->source_factor=units[factor]/192.f;
    return 1;
}
static S1ClockPulse source_clock_sample(original_runtime_arp *a)
{
    const int prev=a->seq.previousHeldNoteNumbersAECount, first=!prev&&a->nheld, on=a->prm.arpIsOn==1;
    S1ClockPulse p={on&&(first||(prev&&!a->nheld)),0,0};
    if (p.reset) a->source_phase=0;
    if (on || (a->seq.sequencerLastNotesCount && !p.reset)) {
        const double r0=__builtin_fmod(a->source_phase,a->source_factor);
        a->source_phase+=a->source_delta;
        const double r1=__builtin_fmod(a->source_phase,a->source_factor);
        p.trigger=first||r1<r0;
        if (p.trigger && on && a->nheld) {
            const double i=a->source_phase/a->source_factor;
            if (i>=2147483648.) p.trigger=0;
            else p.index=(int)i;
        }
    }
    return p;
}
#endif
void original_runtime_arp_clock'''),('        for (unsigned i = 0; i < frames; ++i) {\n            if (S1Sequencer_willFire','''#if X0X_S1_FENCED_CLOCK
        const int source_available=a->source_set && p->kind==ORIGINAL_SOUND_SYNTHONE &&
            !a->clk_beat && a->prm.seqGate==100;
        if (!source_available) a->source_wait=!!a->source_set;
        else if (a->source_wait && !a->nheld) {
            a->source_wait=0; a->source_phase=0; a->prm.clocked=0;
        }
#endif
        for (unsigned i = 0; i < frames; ++i) {
#if X0X_S1_FENCED_CLOCK
            if (source_available && !a->source_wait) {
                const S1ClockPulse pulse=source_clock_sample(a);
                if ((pulse.reset||pulse.trigger) && i>done) {
                    if (render_backend(p,w,left+done,right+done,i-done)!=ORIGINAL_RUNTIME_OK)
                        return p->status;
                    done=i;
                }
                S1Sequencer_processOriginal(&a->seq,&a->prm,a->held,a->nheld,p,arp_key_on,arp_key_off,&pulse);
                if (p->status!=ORIGINAL_RUNTIME_OK) return p->status;
                if (pulse.trigger&&a->seq.gateOpen) { a->fired=1; a->age=0; }
                a->age+=a->age<UINT32_MAX;
                continue;
            }
#endif
            if (S1Sequencer_willFire''')])
e('app/engine.c',[('static original_runtime_part original_bass[NBASS] POOL;','''static original_runtime_part original_bass[NBASS] POOL;
#if X0X_S1_FENCED_CLOCK
/* Serialized owner setup for host audition only. */
int engine_host_s1_clock(unsigned owner, float bpm, unsigned factor)
{
    return owner<NBASS && original_runtime_s1_clock(&original_bass[owner],bpm,factor);
}
#endif''')])
