#include "reader/morph_compute.h"
#include "wave_frequencies.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
static unsigned long long checks=0,cases=0,reads=0;static int observing=0;
static const s1_morph_binding *expected_binding;
static unsigned expected_row[4],expected_sample[4],used[4],observed;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"FAIL morph reader line%d: %s\n",__LINE__,#x);exit(1);}}while(0)
int __real_s1_morph_read_word(const s1_morph_binding *,unsigned,unsigned,uint32_t *);
int __wrap_s1_morph_read_word(const s1_morph_binding*b,unsigned row,unsigned sample,uint32_t*out){
 if(observing){CHECK(b==expected_binding);CHECK(sample<4096);unsigned match=4;for(unsigned i=0;i<4;++i)if(!used[i]&&row==expected_row[i]&&sample==expected_sample[i]){match=i;break;}CHECK(match<4);used[match]=1;++observed;++reads;}
 return __real_s1_morph_read_word(b,row,sample,out);
}
static void compare_case(sp_data *sp,sp_oscmorph2d original,sp_oscmorph2d runtime,const s1_morph_binding *binding,int band,float shape,unsigned pos,unsigned fraction){
 original.enableBandlimit=runtime.enableBandlimit=1;original.bandlimitIndexOverride=runtime.bandlimitIndexOverride=band;original.wtpos=runtime.wtpos=shape;
 original.lphs=runtime.lphs=(int32_t)((pos<<original.tbl[0]->lobits)+fraction);original.freq=runtime.freq=123.25f;original.amp=runtime.amp=.75f;
 float normalized=shape;if(normalized>1.f)normalized-=(int)normalized;const int index=(int)floorf(band*4+normalized*3),second=index>=band*4+3?index:index+1;
 expected_row[0]=expected_row[2]=(unsigned)index;expected_row[1]=expected_row[3]=(unsigned)second;
 expected_sample[0]=expected_sample[1]=pos;expected_sample[2]=expected_sample[3]=(pos+1)%4096;memset(used,0,sizeof used);observed=0;expected_binding=binding;
 float reference=123.f,actual=456.f;CHECK(sp_oscmorph2d_compute(sp,&original,NULL,&reference)==SP_OK);
 observing=1;CHECK(s1_runtime_morph_compute(sp,&runtime,NULL,&actual,binding)==SP_OK);observing=0;
 CHECK(observed==4);for(unsigned i=0;i<4;++i)CHECK(used[i]);CHECK(!memcmp(&reference,&actual,4));
 runtime.tbl=original.tbl;CHECK(!memcmp(&original,&runtime,sizeof original));++cases;
}
static void rejected(sp_data*sp,sp_oscmorph2d osc,const s1_morph_binding*b){sp_oscmorph2d before=osc;float out=123.f;CHECK(s1_runtime_morph_compute(sp,&osc,NULL,&out,b)==SP_NOT_OK);CHECK(out==123.f);CHECK(!memcmp(&before,&osc,sizeof osc));}
int main(int argc,char**argv){
 CHECK(argc==2);const size_t row=4097u*4u;unsigned char*raw=malloc(52u*row),*packed=malloc(S1_MORPH_DIRECT_BYTES);CHECK(raw&&packed);FILE*f=fopen(argv[1],"rb");CHECK(f);CHECK(fread(raw,1,52u*row,f)==52u*row);CHECK(fgetc(f)==EOF);CHECK(!fclose(f));memcpy(packed,raw,48u*row);memcpy(packed+48u*row,raw+50u*row,2u*row);s1_morph_binding binding={0};CHECK(s1_morph_bind_direct(&binding,packed,S1_MORPH_DIRECT_BYTES));
 const unsigned positions[]={0,1,63,64,65,127,128,4094,4095};const float shapes[]={0.f,.125f,.33333334f,.5f,.66666669f,.875f,1.f,1.125f,2.5f};
 for(unsigned r=0;r<3;++r){const int rates[]={44100,48000,96000};sp_data sp={0};sp.sr=rates[r];sp_ftbl original_rows[52]={{0}},runtime_rows[52]={{0}};sp_ftbl *ot[52],*rt[52];
  for(unsigned i=0;i<52;++i){CHECK(sp_ftbl_init(&sp,&original_rows[i],4096)==SP_OK);original_rows[i].tbl=(float*)(raw+i*row);original_rows[i].del=0;runtime_rows[i]=original_rows[i];runtime_rows[i].tbl=NULL;ot[i]=&original_rows[i];rt[i]=&runtime_rows[i];}
  sp_oscmorph2d original={0},runtime={0};CHECK(sp_oscmorph2d_init(&sp,&original,ot,4,13,(float*)wave_frequencies,0)==SP_OK);CHECK(sp_oscmorph2d_init(&sp,&runtime,rt,4,13,(float*)wave_frequencies,0)==SP_OK);
  for(int band=0;band<13;++band)for(unsigned shape=0;shape<9;++shape)for(unsigned p=0;p<9;++p)for(unsigned frac=0;frac<3;++frac){const unsigned fractions[]={0,1,1023};compare_case(&sp,original,runtime,&binding,band,shapes[shape],positions[p],fractions[frac]);}
  s1_morph_binding empty={0};rejected(&sp,runtime,&empty);sp_oscmorph2d bad=runtime;bad.bandlimitIndexOverride=13;bad.enableBandlimit=1;rejected(&sp,bad,&binding);bad=runtime;bad.wtpos=NAN;rejected(&sp,bad,&binding);bad=runtime;bad.wtpos=-.1f;rejected(&sp,bad,&binding);bad=runtime;bad.freq=NAN;rejected(&sp,bad,&binding);bad=runtime;bad.lphs=-1;rejected(&sp,bad,&binding);bad=runtime;bad.nft=5;rejected(&sp,bad,&binding);
 }
 printf("{\"scope\":\"reader-vs-original-morph-component\",\"cases\":%llu,\"checks\":%llu,\"actual_reader_calls\":%llu,\"raw_runtime_payload_pointers\":0,\"failures\":0}\n",cases,checks,reads);free(packed);free(raw);return 0;
}
