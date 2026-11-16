// Source/host composition only. Runtime uses packed aliases; oracle keeps distinct rows.
#define main original_distinct_fixture_main
#include "scheduler_test.cpp"
#undef main
#include <csignal>
#include "reader/morph_compute.h"
#include <sys/wait.h>

// Test-only observation of source-defined selection at real runtime calls.
// This is not a hardware-load trace. The scheduled wrong-alias PCM control
// below independently couples observed selections to real table consumption.
static const sp_data *scheduled_context=nullptr;
static const sp_oscmorph2d *releasing_morph=nullptr;
static unsigned long long scheduled_releasing_alias_calls=0;
static unsigned long long scheduled_calls=0,scheduled_bands[13]{},scheduled_rows[52]{};
static bool oracle_window=false;
static unsigned long long reader_word_calls=0,oracle_reader_calls=0;
static const s1_morph_binding *scheduled_binding=nullptr;
extern "C" int __real_s1_morph_read_word(const s1_morph_binding*,unsigned,unsigned,uint32_t*);
extern "C" int __wrap_s1_morph_read_word(const s1_morph_binding*b,unsigned row,unsigned sample,uint32_t*out){
    if(oracle_window)++oracle_reader_calls;CHECK(!oracle_window);++reader_word_calls;
    return __real_s1_morph_read_word(b,row,sample,out);
}
extern "C" int __real_s1_runtime_morph_compute(sp_data *,sp_oscmorph2d *,SPFLOAT *,SPFLOAT *,const s1_morph_binding *);
extern "C" int __wrap_s1_runtime_morph_compute(sp_data *sp,sp_oscmorph2d *osc,SPFLOAT *in,SPFLOAT *out,const s1_morph_binding *binding){
    if(sp==scheduled_context&&scheduled_context){
        CHECK(binding==scheduled_binding);CHECK(osc->nft==4&&osc->nbl==13);int band=0;
        if(osc->enableBandlimit>0){
            if(osc->bandlimitIndexOverride<0){for(int i=1;i<osc->nbl;++i)if(osc->freq<=osc->fbl[i]){band=i;break;}}
            else {band=static_cast<int>(std::floor(osc->bandlimitIndexOverride));if(band<0)band=0;}
        }
        float position=osc->wtpos;if(position>1.f)position-=static_cast<int>(position);
        const float findex=band*osc->nft+position*(osc->nft-1);const int index=static_cast<int>(std::floor(findex));
        CHECK(band>=0&&band<13&&index>=0&&index<52);++scheduled_calls;++scheduled_bands[band];++scheduled_rows[index];
        if(index<band*osc->nft+osc->nft-1)++scheduled_rows[index+1];
        if(osc==releasing_morph&&(index==48||index==49))++scheduled_releasing_alias_calls;
    }
    return __real_s1_runtime_morph_compute(sp,osc,in,out,binding);
}

struct AliasFixture {
    Fixture &original;
    Tables tables{};
    float *packed=nullptr;size_t bytes=0;
    explicit AliasFixture(Fixture &f):original(f),tables(f.tables) {
        const size_t page=static_cast<size_t>(sysconf(_SC_PAGESIZE));
        bytes=(50*4097*sizeof(float)+page-1)/page*page;
        packed=static_cast<float*>(mmap(nullptr,bytes,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));CHECK(packed!=MAP_FAILED);
        unsigned unique=0;
        for(unsigned i=0;i<52;++i) {
            unsigned u;
            if(i==48)u=44;else if(i==49)u=45;
            else {u=unique++;std::memcpy(packed+u*4097,f.tables.morph[i].samples,4097*sizeof(float));}
            tables.morph[i].samples=packed+u*4097;
            CHECK(!std::memcmp(tables.morph[i].samples,f.tables.morph[i].samples,4097*sizeof(float)));
            CHECK(tables.morph[i].band==i/4&&static_cast<unsigned>(tables.morph[i].shape)==i%4);
            for(unsigned j=0;j<4097;++j){uint32_t a,b;std::memcpy(&a,tables.morph[i].samples+j,4);std::memcpy(&b,f.tables.morph[i].samples+j,4);CHECK(a==b);}
        }
        CHECK(unique==50);CHECK(tables.morph[48].samples==tables.morph[44].samples);CHECK(tables.morph[49].samples==tables.morph[45].samples);
        CHECK(tables.sine==f.tables.sine&&tables.sine_count==4097);
        CHECK(!mprotect(packed,bytes,PROT_READ));
    }
    ~AliasFixture(){CHECK(!munmap(packed,bytes));}
};
static Snapshot alias_capture(const RuntimeState&a){
    // Physical addresses are intentionally different. Every other serialized
    // descriptor field remains exact; bindings/content are checked separately.
    Snapshot z=common_capture(a,false);
    for(unsigned i=0;i<150;++i)if(synthone_ports::selected_slots[i]!=synthone_ports::unsmoothed_slot){z.put(a.compactPorts.target(i).value);record(z,a.compactPorts.port(i).state);}
    return z;
}
static Snapshot alias_capture(const OracleState&a){
    Snapshot z=common_capture(a,false);
    for(unsigned i=0;i<150;++i)if(a.s1p[i].usePortamento){z.put(a.s1p[i].portamentoTarget);record(z,*a.s1p[i].portamento);}
    return z;
}
template<class S>static void alias_traced(const S&s,unsigned phase,unsigned frame,void*context){auto&t=*static_cast<Trace*>(context);CHECK(t.values.size()<t.values.capacity());auto z=alias_capture(s);z.put(phase);z.put(frame);t.values.push_back(z);}
static unsigned long long warmup_frames=0;
static void warm_controls(Scheduler &s,Oracle &o,int rate){
    // Exercise real source/control startup with no notes; do not poke outputs
    // or port histories to force the desired frequency/table selection.
    for(unsigned remaining=static_cast<unsigned>(rate);remaining;){const unsigned n=remaining>512?512:remaining;float sl[512]{},sr[512]{},ol[512]{},orr[512]{};
        {NoHeap guard;CHECK(s.process(n,0,sl,sr));oracle_window=true;o.process(n,0,ol,orr);oracle_window=false;}
        CHECK(!std::memcmp(sl,ol,sizeof sl)&&!std::memcmp(sr,orr,sizeof sr));equal(alias_capture(s),alias_capture(o));remaining-=n;warmup_frames+=n;
    }
}
struct ReaderPair {
    Scheduler s;Oracle o;AliasFixture &fixture;float values[150];
    explicit ReaderPair(AliasFixture &f,int rate):fixture(f){std::memcpy(values,defaults,sizeof values);values[oscBandlimitEnable]=1;values[index1]=.12f;values[index2]=.45f;reinit(rate);}
    void reinit(int rate){NoHeap guard;CHECK(s.init_reader(fixture.tables,fixture.packed,S1_MORPH_DIRECT_BYTES,rate,values,150,fixture.original.tuning,128));CHECK(o.init(fixture.original.tables,rate,values,150,fixture.original.tuning,128));bindings();compare();}
    void bindings()const{
        CHECK(s.using_reader());CHECK(s.reader_binding().bytes==reinterpret_cast<const unsigned char*>(fixture.packed));CHECK(s.reader_binding().byte_count==S1_MORPH_DIRECT_BYTES);CHECK(s1_morph_binding_valid(&s.reader_binding()));
        for(unsigned i=0;i<52;++i){CHECK(s.ft_array[i]->tbl==fixture.tables.morph[i].samples);CHECK(o.ft_array[i]->tbl==fixture.original.tables.morph[i].samples);CHECK(s.ft_array[i]->tbl!=o.ft_array[i]->tbl);CHECK(s.ft_array[i]->del==0&&o.ft_array[i]->del==0);}
        CHECK(s.sine->tbl==fixture.original.tables.sine&&o.sine->tbl==fixture.original.tables.sine);
    }
    void compare()const{equal(alias_capture(s),alias_capture(o));}
    void parameter(unsigned p,float v){NoHeap n;CHECK(s.set_target(p,v));o._setSynthParameter(p,v);++events;compare();}
    void start(int pitch,int velocity){NoHeap n;CHECK(s.start(pitch,velocity));o.startNote(pitch,velocity,1);++events;compare();}
    void on(int pitch,int velocity,float hz){NoHeap n;CHECK(s.on(pitch,velocity,hz));o.turnOnKey(pitch,velocity,hz);++events;compare();}
    void off(int pitch){NoHeap n;CHECK(s.off(pitch));o.turnOffKey(pitch);++events;compare();}
    void reset(){NoHeap n;CHECK(s.reset());o.reset();++events;compare();}
    void restore(){NoHeap n;CHECK(s.restore(values,150));CHECK(o.restore(values,150));++events;compare();}
    void block(unsigned n,unsigned offset=0){
        CHECK(n<=128&&offset<=7);float sl[136],sr[136],ol[136],orr[136];for(unsigned i=0;i<136;++i)sl[i]=sr[i]=ol[i]=orr[i]=123.25f;
        Trace st,ot;st.values.reserve(n+1);ot.values.reserve(n+1);s.trace_fn=alias_traced<RuntimeState>;o.trace_fn=alias_traced<OracleState>;s.trace_context=&st;o.trace_context=&ot;
        {NoHeap guard;CHECK(!scheduled_context);scheduled_context=s.sp;scheduled_binding=&s.reader_binding();CHECK(s.process(n,offset,sl,sr));scheduled_context=nullptr;scheduled_binding=nullptr;oracle_window=true;o.process(n,offset,ol,orr);oracle_window=false;}
        s.trace_fn=nullptr;o.trace_fn=nullptr;s.trace_context=o.trace_context=nullptr;
        CHECK(st.values.size()==n+1&&ot.values.size()==n+1);for(size_t i=0;i<st.values.size();++i)equal(st.values[i],ot.values[i]);
        CHECK(!std::memcmp(sl,ol,sizeof sl)&&!std::memcmp(sr,orr,sizeof sr));for(unsigned i=0;i<n;++i){CHECK(std::isfinite(sl[offset+i]));CHECK(std::isfinite(sr[offset+i]));}frames+=n;bindings();compare();
    }
};
static unsigned write_probes=0,oscillator_cases=0,wrong_aliases_detected=0;
static unsigned scheduled_wrong_aliases_detected=0,scheduled_wrong_alias_frames=0;
static void protected_write(const float*p){
    const pid_t child=fork();CHECK(child>=0);
    if(child==0){std::signal(SIGSEGV,SIG_DFL);std::signal(SIGBUS,SIG_DFL);*const_cast<volatile float*>(p)=17.f;_exit(0);}
    int status=0;CHECK(waitpid(child,&status,0)==child);CHECK(WIFSIGNALED(status));CHECK(WTERMSIG(status)==SIGSEGV||WTERMSIG(status)==SIGBUS);++write_probes;
}
static void oscillator_boundaries(ReaderPair&p){
    p.s.initializeNoteStates();p.o.initializeNoteStates();
    const unsigned positions[]={0,1,63,64,65,127,128,4094,4095};
    const float shapes[]={0.f,.125f,.33333334f,.5f,.66666669f,.875f,1.f};
    for(int band=0;band<13;++band)for(float shape:shapes)for(unsigned pos:positions)for(unsigned fraction:{0u,1u,1023u}){
        auto a=*p.s.poly[0].oscmorph1,b=*p.o.poly[0].oscmorph1;
        a.enableBandlimit=b.enableBandlimit=1;a.bandlimitIndexOverride=b.bandlimitIndexOverride=band;a.wtpos=b.wtpos=shape;a.freq=b.freq=123.25f;a.amp=b.amp=.75f;
        a.lphs=b.lphs=static_cast<int32_t>((pos<<p.s.ft_array[0]->lobits)+fraction);float x=123,y=456;
        {NoHeap n;CHECK(s1_runtime_morph_compute(p.s.sp,&a,nullptr,&x,&p.s.reader_binding())==SP_OK);sp_oscmorph2d_compute(p.o.sp,&b,nullptr,&y);}
        CHECK(!std::memcmp(&x,&y,sizeof x));CHECK(a.lphs==b.lphs&&a.inc==b.inc);++oscillator_cases;
    }
    // A structurally admitted but deliberately corrupted data row changes only
    // the runtime reader; the original C comparator retains canonical words.
    std::vector<float> corrupt(50*4097);
    std::memcpy(corrupt.data(),p.fixture.packed,corrupt.size()*sizeof(float));
    std::memcpy(corrupt.data()+44*4097,corrupt.data()+48*4097,4097*4);
    s1_morph_binding wrong{};CHECK(s1_morph_bind_direct(&wrong,corrupt.data(),corrupt.size()*sizeof(float)));
    auto a=*p.s.poly[0].oscmorph1,b=*p.o.poly[0].oscmorph1;bool differs=false;
    for(unsigned pos:positions){a.enableBandlimit=b.enableBandlimit=1;a.bandlimitIndexOverride=b.bandlimitIndexOverride=12;a.wtpos=b.wtpos=.12f;a.freq=b.freq=123.25f;a.amp=b.amp=.75f;a.lphs=b.lphs=static_cast<int32_t>(pos<<p.s.ft_array[0]->lobits);float x=0,y=0;CHECK(s1_runtime_morph_compute(p.s.sp,&a,nullptr,&x,&wrong)==SP_OK);sp_oscmorph2d_compute(p.o.sp,&b,nullptr,&y);differs=differs||std::memcmp(&x,&y,4);}
    CHECK(differs);++wrong_aliases_detected;
}
static void scheduled_wrong_alias(AliasFixture &f,int rate){
    case_name="scheduled wrong-row48 alias must change actual PCM";++cases;
    std::vector<float> corrupt(50*4097);std::memcpy(corrupt.data(),f.packed,corrupt.size()*sizeof(float));std::memcpy(corrupt.data()+44*4097,corrupt.data()+48*4097,4097*4);
    Tables wrong=f.tables;for(unsigned i=0;i<52;++i){const unsigned u=i<48?i:i==48?44:i==49?45:i-2;wrong.morph[i].samples=corrupt.data()+u*4097;}
    Scheduler broken;Oracle original;float values[150];std::memcpy(values,defaults,sizeof values);
    values[oscBandlimitEnable]=1;values[index1]=.12f;values[index2]=.12f;
    {NoHeap guard;CHECK(broken.init_reader(wrong,corrupt.data(),corrupt.size()*sizeof(float),rate,values,150,f.original.tuning,128));CHECK(original.init(f.original.tables,rate,values,150,f.original.tuning,128));}
    warm_controls(broken,original,rate);
    {NoHeap guard;CHECK(broken.start(127,111));original.startNote(127,111,1);}
    bool differs=false,audible=false;
    for(unsigned pass=0;pass<4;++pass){float bl[128]{},br[128]{},ol[128]{},orr[128]{};
        {NoHeap guard;CHECK(broken.process(128,0,bl,br));oracle_window=true;original.process(128,0,ol,orr);oracle_window=false;}
        for(unsigned i=0;i<128;++i){CHECK(std::isfinite(bl[i])&&std::isfinite(br[i])&&std::isfinite(ol[i])&&std::isfinite(orr[i]));audible=audible||ol[i]!=0||orr[i]!=0;}
        differs=differs||std::memcmp(bl,ol,sizeof bl)||std::memcmp(br,orr,sizeof br);scheduled_wrong_alias_frames+=128;
    }
    CHECK(audible);CHECK(differs);++scheduled_wrong_aliases_detected;
}
static unsigned long long reader_word_comparisons=0;
static void reader_admission(ReaderPair &p,int rate){
    case_name="reader complete-owner admission, mode replacement and output aliases";++cases;
    for(unsigned r=0;r<52;++r)for(unsigned i=0;i<4097;++i){uint32_t actual=0,expected=0;CHECK(s1_morph_read_word(&p.s.reader_binding(),r,i,&actual));std::memcpy(&expected,p.fixture.original.tables.morph[r].samples+i,4);CHECK(actual==expected);++reader_word_comparisons;}
    unsigned char before[sizeof(Scheduler)];std::memcpy(before,&p.s,sizeof p.s);float output=123;auto*late=reinterpret_cast<float*>(const_cast<uint32_t*>(&p.s.reader_binding().format_tag));
    Tables bad=p.fixture.tables;bad.morph[48].samples=bad.morph[50].samples;
    {NoHeap guard;
        CHECK(!p.s.init_reader(p.fixture.tables,&p.s.reader_binding(),S1_MORPH_DIRECT_BYTES,rate,p.values,150,p.fixture.original.tuning,128));
        // Only this storage address may be inspected before owner-overlap rejection.
        CHECK(!p.s.init_reader(*reinterpret_cast<const Tables*>(&p.s.reader_binding()),p.fixture.packed,S1_MORPH_DIRECT_BYTES,rate,p.values,150,p.fixture.original.tuning,128));
        CHECK(!p.s.init_reader(bad,p.fixture.packed,S1_MORPH_DIRECT_BYTES,rate,p.values,150,p.fixture.original.tuning,128));
        CHECK(!p.s.init_reader(p.fixture.tables,p.fixture.packed,S1_MORPH_DIRECT_BYTES-1,rate,p.values,150,p.fixture.original.tuning,128));
        CHECK(!p.s.init_reader(p.fixture.tables,reinterpret_cast<const void*>(UINTPTR_MAX-1),S1_MORPH_DIRECT_BYTES,rate,p.values,150,p.fixture.original.tuning,128));
        CHECK(!p.s.valid_output(late,&output,1,0));CHECK(!p.s.valid_output(const_cast<float*>(p.fixture.packed),&output,1,0));
    }
    CHECK(!std::memcmp(before,&p.s,sizeof p.s));CHECK(output==123);p.bindings();p.compare();
    {NoHeap guard;CHECK(p.s.init(p.fixture.tables,rate,p.values,150,p.fixture.original.tuning,128));}CHECK(!p.s.using_reader());p.reinit(rate);p.bindings();p.compare();
}
int main(int argc,char**argv){
    CHECK(argc==2);Fixture distinct(argv[1]);AliasFixture aliases(distinct);
    case_name="distinct original and separate packed read-only payload";++cases;
    for(const float*p:{aliases.tables.morph[0].samples,aliases.tables.morph[44].samples,aliases.tables.morph[45].samples,aliases.tables.morph[48].samples,aliases.tables.morph[49].samples,aliases.tables.morph[50].samples,aliases.tables.sine})protected_write(p);
    for(int rate:{44100,48000,96000}){
        const auto before48=scheduled_rows[48],before49=scheduled_rows[49];
        case_name="packed runtime vs distinct original scheduler";++cases;ReaderPair p(aliases,rate);reader_admission(p,rate);warm_controls(p.s,p.o,rate);oscillator_boundaries(p);
        for(int i=0;i<6;++i)p.start(122+i,41+i*13);p.block(128,3);p.parameter(index1,.81f);p.parameter(index2,.44f);p.block(65);p.on(127,111,20000.f);p.off(127);CHECK(p.s.poly[5].rootNoteNumber==127&&p.o.poly[5].rootNoteNumber==127);CHECK(p.s.poly[5].stage==S1NoteState::stageRelease&&p.o.poly[5].stage==S1NoteState::stageRelease);const auto release_before=scheduled_releasing_alias_calls;releasing_morph=p.s.poly[5].oscmorph1;p.block(63);releasing_morph=nullptr;CHECK(scheduled_releasing_alias_calls>release_before);p.restore();p.block(0);
        p.parameter(isMono,1);p.start(127,111);p.block(128);p.start(120,63);p.block(64);p.off(120);p.block(64);p.reset();p.block(1);p.reinit(rate);p.start(123,91);p.block(37);
        const auto before_s=alias_capture(p.s),before_o=alias_capture(p.o);{ReaderPair other(aliases,rate);other.start(77,89);other.block(17);equal(before_s,alias_capture(p.s));equal(before_o,alias_capture(p.o));}
        self_bound(p.s);self_bound(p.o);p.bindings();p.compare();
        CHECK(scheduled_rows[48]>before48&&scheduled_rows[49]>before49);scheduled_wrong_alias(aliases,rate);
    }
    std::printf("{\"scope\":\"reader-scheduler-host-composition\",\"cases\":%u,\"checks\":%llu,\"frames\":%llu,\"state_snapshots\":%llu,\"events\":%llu,\"word_comparisons\":213044,\"oscillator_boundary_cases\":%u,\"protected_write_probes\":%u,\"wrong_alias_controls_detected\":%u,\"scheduled_wrong_alias_controls_detected\":%u,\"scheduled_wrong_alias_frames\":%u,\"no_note_warmup_frames\":%llu,\"scheduled_releasing_alias_calls\":%llu,\"scheduled_morph_calls\":%llu,\"scheduled_row48_selections\":%llu,\"scheduled_row49_selections\":%llu,\"reader_word_calls\":%llu,\"oracle_reader_calls\":%llu,\"reader_word_comparisons\":%llu,\"heap_calls_during_actions\":0,\"failures\":0}\n",cases,checks,frames,snapshots,events,oscillator_cases,write_probes,wrong_aliases_detected,scheduled_wrong_aliases_detected,scheduled_wrong_alias_frames,warmup_frames,scheduled_releasing_alias_calls,scheduled_calls,scheduled_rows[48],scheduled_rows[49],reader_word_calls,oracle_reader_calls,reader_word_comparisons);return 0;
}
