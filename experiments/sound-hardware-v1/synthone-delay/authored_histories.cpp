/* Test-only. Original ordered restores, note scheduler, smoothed edits and full FX graph. */
#include "full_fixture.hpp"
#include "oracle_controls.hpp"
#define S1FX_EXISTING_PARAMETERS 1
#include "source_fx.hpp"

constexpr unsigned hz = 44100;
constexpr unsigned long_edit = hz * 4;
constexpr unsigned wet_edit = hz * 11 / 2;
constexpr unsigned end_at = wet_edit + hz / 2;
struct Owner {
    FullPair pair;
    s1frontend_oracle::OracleControls controls;
    SourceFx fx{hz};
    float *left = nullptr, *right = nullptr;
    float mode = 0, peak = 0;
    double tap_energy = 0;
    unsigned absolute = 0, old_reads = 0, nonzero_old_reads = 0, oldest = 0;
    Owner(Fixture& f):pair(f,hz,false,false,0),controls(pair.o){}
    void edit(S1Parameter id,float value){CHECK(controls.set_parameter(id,value));}
    static void capture(const OracleState& s,unsigned phase,unsigned frame,void* self){
        if(phase != 1) return;
        Owner& x = *static_cast<Owner*>(self);
        if(x.mode != s.parameters[isMono]) {
            x.mode=s.parameters[isMono];
            x.fx.source_reset_delays();
        }
        std::memcpy(x.fx.parameters.data(),s.parameters,sizeof(float)*150);
        x.fx.frame(x.left[frame],x.left+frame,x.right+frame,
                   s.lfo1_0_1,s.lfo1_1_0,s.lfo2_0_1,s.lfo2_1_0);
        const unsigned now = x.absolute + frame;
        const unsigned lag = static_cast<unsigned>(x.fx.delayL->del * hz);
        if(now < wet_edit || x.fx.delayCrossfadeL->pos <= .9f ||
           lag > now || now - lag + 3 >= long_edit) return;
        ++x.old_reads;
        const float tap=x.fx.delayL->prev;
        CHECK(std::isfinite(tap) && std::isfinite(x.left[frame]));
        x.nonzero_old_reads += tap != 0;
        x.oldest=std::max(x.oldest,lag);
        x.peak=std::max(x.peak,std::fabs(tap));
        x.tap_energy+=double(tap)*tap;
    }
    void block(unsigned start,unsigned n) {
        absolute = start;
        float l[128]{},r[128]{};
        left=l;right=r;
        pair.o.trace_fn=capture;pair.o.trace_context=this;
        compact_oracle_window=true;pair.o.process(n,0,l,r);compact_oracle_window=false;
        pair.o.trace_fn=nullptr;
        pair.o.trace_context=nullptr;
    }
};

static unsigned read_word(FILE* f){
    unsigned word;
    CHECK(std::fread(&word,sizeof word,1,f)==1);
    return word;
}
int main(int argc,char**argv){
    CHECK(argc==4);
    Fixture fixture(argv[1]);
    FILE* f=std::fopen(argv[2],"rb");
    CHECK(f);
    std::vector<unsigned char> pack(165292);
    CHECK(std::fread(pack.data(),1,pack.size(),f)==pack.size() && std::fclose(f)==0);
    compact_data=pack.data();compact_size=pack.size();
    f=std::fopen(argv[3],"rb");
    CHECK(f && read_word(f)==0x53315043 && read_word(f)==694);
    Owner bass(fixture),square(fixture);
    Owner* owners[]={&bass,&square};
    const unsigned rows[]={1,8};
    CHECK(bass.fx.delayL->buf.ptr!=square.fx.delayL->buf.ptr);
    for(unsigned row=0; row<694; ++row) {
        const unsigned n=read_word(f);
        CHECK(n==144);
        for(unsigned i=0;i<n;++i) {
            const unsigned id=read_word(f);
            float value;
            CHECK(std::fread(&value,sizeof value,1,f)==1);
            for(unsigned owner=0;owner<2;++owner) {
                if(row==rows[owner]) CHECK(owners[owner]->controls.set_parameter(id,value));
            }
        }
    }
    CHECK(std::fgetc(f)==EOF && !std::ferror(f) && std::fclose(f)==0);
    for(auto* x:owners) {
        CHECK(x->controls.target(delayOn)==0);
        x->pair.o.resetSequencer();
        x->edit(tempoSyncToArpRate,0);
        x->edit(delayTime,minimums[delayTime]);
        x->edit(delayMix,0);
        x->edit(delayFeedback,0);
    }
    for(unsigned pos=0;pos<end_at;) {
        for(unsigned p=0;p<2;++p) {
            Owner& x=*owners[p];
            if(pos%11025==0) {
                x.pair.o.stopAllNotes();
                if(pos<long_edit) x.pair.o.startNote(42+int(p)*17+int(pos/11025)%5,83+int(p)*23);
            }
            if(pos==long_edit) x.edit(delayTime,maximums[delayTime]);
            if(pos==wet_edit) {x.edit(delayOn,1);x.edit(delayMix,1);}
        }
        unsigned n=std::min(128u,std::min(end_at-pos,11025-pos%11025));
        for(auto* x:owners) x->block(pos,n);
        pos+=n;
    }
    std::puts("program_index,wet_reads_of_pre_edit_history,nonzero_reads,oldest_lag_frames,peak_left_delayed_sample,rms_left_delayed_sample");
    for(unsigned p=0;p<2;++p) {
        const Owner& x=*owners[p];
        CHECK(x.old_reads>1000 && x.nonzero_old_reads>1000 && x.oldest>4*hz && x.peak>.001f);
        std::printf("%u,%u,%u,%u,%.9g,%.12g\n",rows[p],x.old_reads,x.nonzero_old_reads,
                    x.oldest,x.peak,std::sqrt(x.tap_energy/x.old_reads));
    }
}
