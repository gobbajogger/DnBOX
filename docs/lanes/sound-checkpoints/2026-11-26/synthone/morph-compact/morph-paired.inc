/* Host diagnostic work reduction, same values/storage as shared-tight. */
typedef struct {float a,b;} sample_pair;
static unsigned fold_index(unsigned shape,unsigned p,float *sign)
{
    *sign=1.f;
    if(shape<2u){
        if(p>=2048u){p-=2048u;*sign=-1.f;}
        if(p>1024u){p=2048u-p;if(shape==0u)*sign=-*sign;}
    }else if(shape==3u&&p>2048u){p=4096u-p;*sign=-1.f;}
    return p;
}
static int residual_at(const uint8_t *b,unsigned width,unsigned j)
{
    if(!width)return 0;
    unsigned bit=width*j;
    const uint8_t *x=b+(bit>>3);
    unsigned z=((x[0] | (unsigned)x[1]<<8 | (unsigned)x[2]<<16)>>(bit&7u)) & ((1u<<width)-1u);
    return (int)(z>>1)^-(int)(z&1u);
}
static float cubic_at(float a,float b,float c,float d,float f)
{return b+.5f*f*(c-a+f*(2*a-5*b+4*c-d+f*(3*(b-c)-a+d)));}
static sample_pair stored_pair(unsigned row,unsigned p)
{
    const unsigned q=(p+1u)&4095u;
    if(row<4u)return (sample_pair){stored_lattice(row,p),stored_lattice(row,q)};
    float sa,sb;
    unsigned x=fold_index(row&3u,p,&sa), y=fold_index(row&3u,q,&sb);
    unsigned r=row-4,shift=morph_rows[r].shift, k=x>>shift;
    if (k!=(y>>shift)) return (sample_pair){stored_lattice(row,p),stored_lattice(row,q)};
    unsigned span=(1u<<shift),u=x&(span-1u),v=y&(span-1u);
    const uint8_t *data=morph_bytes+morph_rows[r].at;
    float aa,bb;
    if(morph_rows[r].mode==0){
        const uint8_t *b=data+morph_rows[r].index_bytes+u16(data+2*k);
        unsigned bw=*b;
        if(bw==16){aa=s16(b+1+2*u);bb=s16(b+1+2*v);}
        else {
            int org=(int16_t)u16(b+1), step=(int16_t)u16(b+3);
            aa=(float)(org+(int)u*step+residual_at(b+5,bw,u));
            bb=(float)(org+(int)v*step+residual_at(b+5,bw,v));
        }
    }else{
        data+=2*k;
        float f=(float)u/(float)span,g=(float)v/(float)span;
        const float a=s16(data),b=s16(data+2);
        if(morph_rows[r].mode==1){
            aa=a+(b-a)*f;bb=a+(b-a)*g;
        }else{
            const float c=s16(data+4),d=s16(data+6);
            aa=cubic_at(a,b,c,d,f);bb=cubic_at(a,b,c,d,g);
        }
    }
    return (sample_pair){sa*aa*(1.f/16384.f),sb*bb*(1.f/16384.f)};
}
static sample_pair shared_shape(unsigned row,unsigned p,sample_pair saw,unsigned kind)
{
    if(kind==0)return stored_pair(row&~3u,p);
    if(kind==3)return saw;
    if(kind==1){
        sample_pair h=stored_pair(row|3u,(p-2048u)&4095u);
        return (sample_pair){(40.f/39.f)*(saw.a-h.a),(40.f/39.f)*(saw.b-h.b)};
    }
    sample_pair h=stored_pair(row|3u,(p-512u)&4095u);
    return (sample_pair){.75f+(3.2f/3.14159265358979323846f)*(saw.a-h.a),
                         .75f+(3.2f/3.14159265358979323846f)*(saw.b-h.b)};
}
static sample_pair mixed_pair(unsigned row,unsigned row2,unsigned p,float mix)
{
    sample_pair a,b;
#if DNBX_MORPH_SAW_SHARE
    if(row>=4u){
        sample_pair saw=stored_pair(row|3u,p);
        a=shared_shape(row,p,saw,row&3u);
        b=row2==row?a:shared_shape(row,p,saw,row2&3u);
    }else
#endif
    {a=stored_pair(row,p);b=row2==row?a:stored_pair(row2,p);}
    return (sample_pair){(1.f-mix)*a.a+mix*b.a,(1.f-mix)*a.b+mix*b.b};
}
