#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef struct {float s1,s2,s3,s4,thr;} old_coeff;
typedef struct {float s1,s2,s3,s4,thr,a,b;} new_coeff;
void old_coef(old_coeff *,float,float,float,float,float);
void new_coef(new_coeff *,float,float,float,float,float);
float old_comp(float *,float,const old_coeff *,int);
float new_comp(float *,float,const new_coeff *,int);
static uint32_t r=0xa7c024ddu;
static float unit(void){
    r^=r<<13; r^=r>>17; r^=r<<5;
    return (float)(r>>8)*(1.0f/16777216.0f);
}
int main(void){
    old_coeff oc;
    new_coeff nc;
    float a[4]={0,0,0,1},b[4]={0,0,0,1};
    for(unsigned i=0;i<1048576u;++i){
        if(i%113==0){
            const float ratio=1.f+unit()*19.f, threshold=-80.f*unit();
            const float atk=i%2 ? .001f+unit() : 0.f, rel=i%3 ? .001f+unit() : 0.f;
            const float rate=i%4 ? 44100.f : 48000.f;
            old_coef(&oc,ratio,threshold,atk,rel,rate);
            new_coef(&nc,ratio,threshold,atk,rel,rate);
            if(memcmp(&oc,&nc,sizeof oc)) return 1;
        }
        float x=i%257u==0 ? -0.f : (2.f*unit()-1.f)*unit();
        int upd=i%13==0 || i%4==0;
        const float y=old_comp(a,x,&oc,upd), z=new_comp(b,x,&nc,upd);
        if(memcmp(a,b,sizeof a) || memcmp(&y,&z,sizeof y)){
            fprintf(stderr,"compressor output/state mismatch at %u (%a/%a)\n",i,y,z);return 1;
        }
    }
    puts("1,048,576 exact compressor samples and states; changed coefficients/quarter cadence/zero attack/release");
}
