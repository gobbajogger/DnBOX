/* Diagnostic coefficient preparer for neutral physical integer cutoff only. */
float normal_sine2048(unsigned i)
{
    i&=2047u;
    if(i==1024u)return 0x1.1a6264p-53f;
    unsigned q=i&511u;
    if(i&512u)q=512u-q;
    const F a=mul(f((float)q),(F){0x1.921fb6p-9f,-0x1.777a5cp-34f});
    const float v=rd(mul(a,eval(mul(a,a),sn,10)));
    return i&1024u?-v:v;
}
static float normal_fast_lookup(float p)
{
    float pos=p*2048.f;unsigned i=(unsigned)pos;
    float t=pos-(float)i;
    return rd(add(mul(sum(1.f,-t),f(normal_sine2048(i))),f(normal_sine2048(i+1u)*t)));
}
int normal_prepare_neutral(unsigned rate,unsigned physical,F c[5])
{
    if((rate!=44100u && rate!=48000u) || physical<12u || physical>30000u)return 0;
    float p=mn((float)physical/(float)rate,.495f);
    const float s=normal_fast_lookup(p), co=normal_fast_lookup(p+.25f);
    const F beta=div2(mul(sum(1.f,-s),f(.5f)),sum(1.f,s));
    const F u=add(beta,f(.5f)), v=mul(u,f(co));
    const F t=mul(add(u,neg(v)),f(.25f));
    c[0]=(F){t.h*2.f,t.l*2.f}; c[1]=(F){t.h*4.f,t.l*4.f}; c[2]=c[0];
    c[3]=(F){-2.f*v.h,-2.f*v.l}; c[4]=(F){2.f*beta.h,2.f*beta.l};
    return 1;
}
