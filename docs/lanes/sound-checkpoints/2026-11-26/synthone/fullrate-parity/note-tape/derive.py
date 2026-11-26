"""Derive host observer/replayer from frozen source; no product edits."""
from pathlib import Path
import sys
root=Path(__file__).resolve().parent.parent
out=Path(sys.argv[1]); out.mkdir(exist_ok=True)
cand=Path(sys.argv[2])
def sub(text,before,after):
    assert text.count(before)==1,(before,text.count(before));return text.replace(before,after)
orig=(root/'s1-fx-parity/original.cpp').read_text()
orig=sub(orig,"unsigned mode; };","unsigned mode; unsigned frame; };")
orig=sub(orig,"auto&ctx=*static_cast<Context*>(v);auto&f=*ctx.fx;","auto&ctx=*static_cast<Context*>(v);auto&f=*ctx.fx;\n    ++ctx.frame;")
orig=sub(orig,"CHECK(argc==6);","CHECK(argc==7);")
orig=sub(orig,"Context ctx{&fx,nullptr,nullptr,0.f,getmode(argv[5])};","""Context ctx{&fx,nullptr,nullptr,0.f,getmode(argv[5]),0};
    FILE *tape=std::fopen(argv[6],"wb");CHECK(tape);
    const bool active=o.parameters[arpIsOn]==1.f;
    std::fprintf(tape,"S1T1 %u %u %u %.9g\\n",want,S1AB_LEAD+S1AB_FRAMES,unsigned(active),o.parameters[arpIsSequencer]);
    o.sequencer.on=[&](int n,int v) {
        if(n>=0&&n<128){CHECK(v>0&&v<128);std::fprintf(tape,"%u %d %d\\n",ctx.frame,n,v);}
        o.turnOnKey(n,v);
    };
    o.sequencer.off=[&](int n) {
        if(n>=0&&n<128)std::fprintf(tape,"%u %d 0\\n",ctx.frame,n);
        o.turnOffKey(n);
    };""")
orig=sub(orig,"CHECK(!std::fclose(out));","CHECK(!std::fclose(out));CHECK(!std::fclose(tape));")
(out/'orig.cpp').write_text(orig)
drv=(cand/'render.c').read_text()
drv=sub(drv,"static const unsigned notes[]","""typedef struct { unsigned frame, pitch, velocity; } tape_event;
static tape_event *event;
static unsigned event_count, cursor, frame_counter;
static FILE *witness;
void s1_external_tape_frame(s1c *core)
{
    if (core!=&part.e) abort();
    while (cursor<event_count && event[cursor].frame==frame_counter) {
        const tape_event *e=&event[cursor++];
        if(!s1_part_note(&part,e->pitch,e->velocity)) abort();
        if(witness) fprintf(witness,"%u %u %u\\n",frame_counter,e->pitch,e->velocity);
    }
    ++frame_counter;
}
static const unsigned notes[]""")
drv=sub(drv,"if(argc!=5)return 2;","if(argc!=5&&argc!=7)return 2;")
drv=sub(drv,"s1_part_init(&part);","""unsigned active=0,total=0;
    if (argc==7) {
        unsigned tapepreset=0;
        float seq=0;
        FILE *in=fopen(argv[5],"rb");
        if(!in||fscanf(in,"S1T1 %u %u %u %f",&tapepreset,&total,&active,&seq)!=4||tapepreset!=index||active>1||total!=264728) return 6;
        tape_event e;
        int count;
        while((count=fscanf(in,"%u %u %u",&e.frame,&e.pitch,&e.velocity))==3){
            if(!active||e.frame>=total||e.pitch>127||e.velocity>127||(event_count&&e.frame<event[event_count-1].frame))return 6;
            event=realloc(event,(event_count+1)*sizeof *event);
            if(!event) return 6;
            event[event_count++]=e;
        }
        if(count!=EOF||fclose(in))return 6;
        witness=fopen(argv[6],"wb"); if(!witness)return 6;
        fprintf(witness,"S1T1 %u %u %u %.9g\\n",index,total,active,seq);
    }
    s1_part_init(&part);""")
drv=sub(drv,"s1_part_note(&part,notes[i],64+11*i);","if(!active) s1_part_note(&part,notes[i],64+11*i);")
drv=sub(drv,"s1_part_note(&part,notes[i],0);","if(!active) s1_part_note(&part,notes[i],0);")
drv=sub(drv,"free(pack);","""free(pack);
    free(event);
    if(argc==7&&(frame_counter!=total||cursor!=event_count||fclose(witness)))return 7;""")
(out/'render.c').write_text(drv)
core=(cand/'dsp/s1c/s1c.c').read_text()
core=sub(core,"void s1c_render(s1c *s, float *outl, float *outr, unsigned n)","extern void s1_external_tape_frame(s1c *);\nvoid s1c_render(s1c *s, float *outl, float *outr, unsigned n)")
core=sub(core,"        sequencer(s);\n#endif","        sequencer(s);\n#else\n        s1_external_tape_frame(s);\n#endif")
(out/'s1c.c').write_text(core)
