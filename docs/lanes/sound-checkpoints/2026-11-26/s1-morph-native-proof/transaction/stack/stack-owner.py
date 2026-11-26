from pathlib import Path
import sys
R=Path(sys.argv[1])/'tree/firmware/src'
def edit(path, pairs):
 p=R/path; t=p.read_text()
 for a,b in pairs:
  assert a in t,(path,a)
  t=t.replace(a,b)
 p.write_text(t)
edit('dsp/drum909.h', [('unsigned, d9_future_t *);','unsigned, d9_future_t *volatile *);')])
edit('dsp/drum909.c', [
 ('unsigned wanted, d9_future_t *future)','unsigned wanted, d9_future_t *volatile *pending)'),
 ('''            unsigned count=0;
            if (future && wanted==order[k]) {
                plat_cpu2_sync();
                count=future->frames;
            }
            if (count) {
                plat_cpu2_sync();
                *s=future->voice;
                bus.y=future->pcm;
                d9_emit_block(&bus,(int)count);''','''            d9_future_t *future=0;
            if (pending && wanted==order[k]) {
                plat_cpu2_sync();
                future=*pending;
            }
            if (future) {
                plat_cpu2_sync();
                *s=future->voice;
                bus.y=future->pcm;
                d9_emit_block(&bus,(int)future->frames);'''),
 ('int cleared, unsigned v, d9_future_t *future)','int cleared, unsigned v, d9_future_t *volatile *pending)'),
 ('n, audible_tracks,v,future);','n, audible_tracks,v,pending);'),
 ('unsigned v,d9_future_t *future)','unsigned v,d9_future_t *volatile *pending)'),
 ('n,mask,1,v,future);','n,mask,1,v,pending);')])
edit('app/engine.c', [
 ('static d9_future_t *eng_d9_future;','static d9_smp_t *eng_d9_seed;\nstatic d9_future_t *volatile eng_d9_ready;'),
 ('eng_d9_voice,eng_d9_future);','eng_d9_voice,&eng_d9_ready);'),
 ('''    if (eng_d9_future)
        drum909_future_run(eng_d9_future,(int)(eng_job_arg & 0xffu));
    if (eng_808_split && !eng_808_body)
        d808_body_run();
    /* The selected workspace is owned by the next worker bass. 909 never
     * waits and can discard unfinished work; stage 1 releases its reader.
     * Existing c1_take_over waits/stops this worker before reclaiming it. */
    if (eng_d9_future && stage_wait(1,1))
        return;''','''    d9_future_t future;
#ifndef X0X_HOST
    if (eng_d9_seed) {
        future.voice=*eng_d9_seed;
        drum909_future_run(&future,(int)(eng_job_arg & 0xffu));
        eng_sync();
        eng_d9_ready=&future;
        eng_sync();
    }
#endif
    /* Real platform: the worker keeps this frame until all its bass jobs
     * finish after stage 1. Existing takeover cannot enter before stage 1.
     * Host runs core1 synchronously, so it leaves this opt-in unadvertised. */
    if (eng_808_split && !eng_808_body)
        d808_body_run();
    (void)future;'''),
 ('    eng_d9_future=0;','    eng_d9_seed=0;\n    eng_d9_ready=0;'),
 ('f->voice=*s; f->frames=0;\n            eng_d9_voice=(uint8_t)v; eng_d9_future=f;',
  'f->voice=*s;\n            eng_d9_voice=(uint8_t)v; eng_d9_seed=&f->voice;'),
 ('        eng_d9_future = 0;','        eng_d9_seed = 0;')])
