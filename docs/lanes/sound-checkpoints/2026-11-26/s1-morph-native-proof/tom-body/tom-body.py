from pathlib import Path
import sys
p=Path(sys.argv[1])/'tree/firmware/src/dsp/drum909.c';t=p.read_text()
a='''    for (int i = 0; i < m; ++i) {
        float f, body, part1 = 0.0f, part2 = 0.0f, stick_amp = 0.0f;'''
b='''    /* The three finite attack lanes have expired. Pitch/body may still run
     * for the entire span. Their strict >m admission never crosses v1; only
     * the final returned cur is visible. No skipped phase, filter or mix. */
    if (m>0 && partial1_ticks==0 && partial2_ticks==0 && stick_ticks==0 &&
        pitch_ticks>m && body_ticks>m) {
        float f=t->pitch.v, body=t->env[0].v, lastf=f, lastbody=body;
        uint32_t ph=t->ph[0];
        for(int i=0;i<m;++i){
            lastf=f; lastbody=body;
            ph+=d9_inc(f);
            float o=0.0f;
            o+=d9_triangle20(ph)*body;
            o*=0.55f;
            float y=d9_shape_v(&p->shape,o+0.0f,t->crush_st);
            y=d9_biquad_tick(&t->dc_block,y);
            bus->y[i]=y*t->out_gain;
            f*=t->pitch.r; body*=t->env[0].r;
        }
        t->ph[0]=ph;
        t->pitch.cur=lastf; t->pitch.v=f;
        t->env[0].cur=lastbody; t->env[0].v=body;
    } else for (int i = 0; i < m; ++i) {
        float f, body, part1 = 0.0f, part2 = 0.0f, stick_amp = 0.0f;'''
assert t.count(a)==1
p.write_text(t.replace(a,b))
