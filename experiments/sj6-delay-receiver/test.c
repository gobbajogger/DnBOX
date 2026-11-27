#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "juno6.h"
#include "fastmath.h"
#include "receiver.inc"
static juno6_t s={.typed=1,.sample_rate=44100.f};
static sj6d_lane l={.epoch=7};
static int16_t a[2048], b[2048];
static float x[65536],y[65536], xr[65536],yr[65536];
static juno6_t ref={.typed=1,.sample_rate=44100.f};
static int16_t ref_ring[2048];
static void check_clock_law(void)
{
    static const float tempi[]={20.f,40.f,57.25f,90.f,120.f,127.f,185.25f,260.f,400.f};
    static const unsigned rates[]={44100,48000};
    static const double grid[]={1./64,3./128,1./32,3./64,1./16,3./32,1./8,3./16,
        1./4,3./8,1./2,3./4,1.,2.,3.,4.,6.,8.};
    for(unsigned r=0;r<2;++r) for(unsigned k=0;k<9;++k) for(unsigned raw=0;raw<128;++raw) {
        juno6_t clock={.sample_rate=(float)rates[r]};
        float free_frames=((1.5f/127.f)*(float)raw)*clock.sample_rate;
        assert(native_delay_frames(&clock,raw,tempi[k],0)==(free_frames<1.f?1.f:free_frames));
        unsigned i=(unsigned)(.5+17.*raw/127.);
        double sec=240./(double)tempi[k]*grid[i];
        double expected=(sec>3.?3.:sec)*rates[r];
        double err=(double)native_delay_frames(&clock,raw,tempi[k],1)-expected;
        assert(err>-.03 && err<.03);
    }
    puts("4608 clock cases: unchanged Free mapping and original Sync mapping");
}
int main(void){
    check_clock_law();
    const uint8_t q[]={3,42,63,64}, bad[]={3,42,63,128}, slow[]={3,127,63,64},off[]={0,42,63,64};
    memset(b,1,sizeof b);
    assert(!s.delay24.tap && sj6d_read(&l,24)==-1);
    assert(sj6d_accept(&l,&s,a,6,1,q,4,120)==SJ6D_STALE);
    assert(sj6d_accept(&l,&s,a,7,1,bad,4,120)==SJ6D_BAD);
    assert(sj6d_accept(&l,&s,a,7,1,slow,4,0)==SJ6D_RANGE);
    assert(!s.delay24.tap && !l.present);
    assert(sj6d_accept(&l,&s,a,7,1,q,4,120)==SJ6D_APPLIED);
    assert(sj6d_read(&l,24)==1 && sj6d_read(&l,118)==1 && sj6d_read(&l,25)==42 && sj6d_read(&l,26)==63 && sj6d_read(&l,27)==64);
    x[16384]=xr[16384]=.5f;y[16384]=yr[16384]=-.25f;
    juno6_delay_native_arm(&ref,ref_ring);
    juno6_delay_native_config(&ref,q,120);
    juno6_delay24_process(&ref,xr,yr,65536);
    juno6_delay24_process(&s,x,y,65536);
    assert(!memcmp(x,xr,sizeof x) && !memcmp(y,yr,sizeof y) && !memcmp(a,ref_ring,sizeof a));
    unsigned wet=0;for(unsigned i=16385;i<65536;i++) wet+=x[i]!=0.f || y[i]!=0.f;
    assert(wet==220);
    assert(sj6d_accept(&l,&s,b,7,1,off,4,120)==SJ6D_UNAVAILABLE);
    sj6d_tick(&l,&s,0); assert(s.delay24.clock_refused && !memcmp(q,l.q,4));
    x[0]=.125f;y[0]=-.5f;
    juno6_delay24_process(&s,x,y,1); assert(x[0]==.125f && y[0]==-.5f);
    sj6d_tick(&l,&s,120); assert(!s.delay24.clock_refused && !memcmp(q,l.q,4));
    assert(sj6d_accept(&l,&s,a,7,1,off,4,120)==SJ6D_APPLIED && sj6d_read(&l,24)==0);
    const uint8_t *c=(const uint8_t *)b;for(unsigned i=0;i<sizeof b;i++) assert(c[i]==1);
    printf("131072 signed reference words bit-exact; compact kernel: %u post-impulse wet frames; range, readback, dry fallback, and isolated alternate arena passed\n",wet);
}
