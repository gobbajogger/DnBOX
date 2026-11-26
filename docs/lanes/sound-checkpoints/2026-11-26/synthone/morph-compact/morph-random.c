/* Static diagnostic alternative to exact-morph.c; no malloc or mutable table. */
#include "exact-morph.h"
#ifdef DNBX_MORPH_TARGET_FRAGMENT
extern long lrintf(float); /* static target inspect, not a linked platform backend */
#else
#include <math.h>
#endif
#include <stdint.h>
#include "morph-data.h"
static unsigned u16(const uint8_t *p) {return p[0] | (unsigned)p[1]<<8;}
static float s16(const uint8_t *p) {return (float)(int16_t)u16(p);}
static float stored_lattice(unsigned row, unsigned p)
{
    const unsigned shape=row&3u;
    if(row<4){
        if(shape==0)return (float)(p<=2048u?p:4096u-p)*(.0009765625f)-1.f;
        if(shape==1)return p<2048u?-1.f:1.f;
        if(shape==2)return p<512u?-1.f:1.f;
        return (float)p*(.00048828125f)-1.f;
    }
    float sign=1.f;
    if(shape<2){
        if(p>=2048u){p-=2048u; sign=-1.f;}
        if(p>1024u){p=2048u-p;if(shape==0)sign=-sign;}
    }else if(shape==3&&p>2048u){p=4096u-p;sign=-1.f;}
    unsigned r=row-4,shift=morph_rows[r].shift, k=p>>shift, j=p&((1u<<shift)-1u);
    const uint8_t *data=morph_bytes+morph_rows[r].at;
    float y;
    if(morph_rows[r].mode==0){
        const uint8_t *b=data+morph_rows[r].index_bytes+u16(data+2*k);
        unsigned bw=*b;
        if(bw==16) y=s16(b+1+2*j);
        else {
            const unsigned bit=bw*j;
            unsigned z=0;
            if(bw){
                const uint8_t *x=b+5+(bit>>3);
                z=(x[0] | (unsigned)x[1]<<8 | (unsigned)x[2]<<16)>>(bit&7u);
                z&=(1u<<bw)-1u;
            }
            int err=(int)(z>>1)^-(int)(z&1u);
            y=(float)((int16_t)u16(b+1)+(int)j*(int16_t)u16(b+3)+err);
        }
    } else {
        const float f=(float)j/(float)(1u<<shift);
        data+=2*k;
        if(morph_rows[r].mode==1){
            const float a=s16(data), b=s16(data+2);
            y=a+(b-a)*f;
        } else {
            const float a=s16(data),b=s16(data+2),c=s16(data+4),d=s16(data+6);
            y=b+.5f*f*(c-a+f*(2*a-5*b+4*c-d+f*(3*(b-c)-a+d)));
        }
    }
    return sign*y*(1.f/16384.f);
}
static float lattice(unsigned row,unsigned p)
{
#if DNBX_MORPH_SAW_SHARE
    if(row>=4){
        const unsigned r=row&3u;
        const unsigned saw=row|3u;
        if(r==1) return (40.f/39.f)*(stored_lattice(saw,p)-stored_lattice(saw,(p-2048u)&4095u));
        if(r==2) return .75f+(3.2f/3.14159265358979323846f)*
                             (stored_lattice(saw,p)-stored_lattice(saw,(p-512u)&4095u));
    }
#endif
    return stored_lattice(row,p);
}
#ifdef DNBX_MORPH_LATTICE_TEST
float morph_lattice_test(unsigned row,unsigned p){return lattice(row,p);}
unsigned morph_rodata_bytes(void){return sizeof morph_bytes+sizeof morph_rows;}
#endif
#if DNBX_MORPH_PAIR
#include "morph-paired.inc"
#endif
int exact_morph_load(void) {return 1;}
float exact_morph_sample(uint32_t *phase,float freq,float wtpos,int bandlimited) {
    static const float limits[13]={0.f,8.178783416748047f,16.357566833496094f,32.71513366699219f,
        65.43026733398438f,131.25f,262.5f,525.f,1050.f,2205.f,4410.f,11025.f,22050.f};
    unsigned band=0;
    if(bandlimited)for(unsigned i=1;i<13;++i)if(freq<=limits[i]){band=i;break;}
    if(wtpos>1.f)wtpos-=(int)wtpos;
    const float wave_pos=(float)(band*4u)+wtpos*3.f;
    const unsigned row=(unsigned)wave_pos;
    const float mix=wave_pos-(float)row;
    const unsigned row2=row>=band*4u+3u?row:row+1u;
    const unsigned p=*phase>>12,q=(p+1u)&4095u;
    const float frac=(float)(*phase&4095u)*(1.f/4096.f);
#if DNBX_MORPH_PAIR
    const sample_pair v=mixed_pair(row,row2,p,mix);
    const float v1=v.a,v2=v.b;
    (void)q;
#else
    const float v1=(1.f-mix)*lattice(row,p)+mix*lattice(row2,p);
    const float v2=(1.f-mix)*lattice(row,q)+mix*lattice(row2,q);
#endif
    *phase=(*phase+(uint32_t)lrintf(freq*(16777216.f/44100.f)))&0xffffffu;
    return v1+(v2-v1)*frac;
}
