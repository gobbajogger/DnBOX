#include "sj6_controls.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
int main(void){
 unsigned char blob[DNBOX_SJ6_BLOB_BYTES],saved[DNBOX_SJ6_BLOB_BYTES];float values[DNBOX_SJ6_NATIVE_COUNT]={0},v=-1;unsigned seen[218]={0};
 assert(DNBOX_SJ6_PAGE_COUNT==8&&DNBOX_SJ6_BLOB_BYTES==888);
 for(unsigned p=0;p<8;p++)for(unsigned s=0;s<8;s++){
  const dnbox_sj6_control*d=dnbox_sj6_control_at(p,s);assert(d&&d->native_id<218&&!seen[d->native_id]++&&d->identifier&&d->units&&d->law);
  assert(dnbox_sj6_binding_status(d->native_id)==DNBOX_CTL_NOT_ADMITTED);
 }
 for(unsigned i=0;i<150;i++){const dnbox_s1_control*d=dnbox_s1_control_at(i);assert(d&&d->native_id==i&&d->identifier&&d->units);}
 assert(!dnbox_s1_control_at(150));
 assert(!dnbox_sj6_control_at(8,0)&&!dnbox_sj6_control_at(0,8));
 for(unsigned i=0;i<218;i++)values[i]=(float)i/218;
 assert(dnbox_sj6_blob_encode(blob,sizeof blob,values,218)==DNBOX_CTL_OK);memcpy(saved,blob,sizeof blob);
 for(unsigned i=0;i<218;i++){assert(dnbox_sj6_blob_get(blob,sizeof blob,i,&v)==DNBOX_CTL_OK&&v==values[i]);}
 for(unsigned i=0;i<sizeof blob;i++){blob[i]^=1;assert(dnbox_sj6_blob_get(blob,sizeof blob,0,&v)==DNBOX_CTL_BAD_BLOB);blob[i]^=1;}
 assert(dnbox_sj6_blob_set(blob,sizeof blob,82,.6666667f)==DNBOX_CTL_OK);
 assert(dnbox_sj6_blob_get(blob,sizeof blob,82,&v)==DNBOX_CTL_OK&&v==.6666667f);
 memcpy(saved,blob,sizeof blob);assert(dnbox_sj6_blob_set(blob,sizeof blob,82,NAN)==DNBOX_CTL_BAD_ARGUMENT);assert(!memcmp(saved,blob,sizeof blob));
 assert(dnbox_sj6_blob_get(blob,sizeof blob,218,&v)==DNBOX_CTL_BAD_ID);
 assert(dnbox_sj6_blob_get(blob,sizeof blob-1,0,&v)==DNBOX_CTL_BAD_BLOB);
 assert(dnbox_sj6_blob_encode(blob,sizeof blob,(float*)blob,218)==DNBOX_CTL_BAD_ARGUMENT);
 puts("PASS:64 unique source descriptors,218-word blob roundtrip,888 corrupt-byte rejections,finite/alias/transaction checks; DSP bindings explicitly NOT_ADMITTED");return 0;
}
