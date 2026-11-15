/* Source-host qualification only; never a device writer. */
#ifndef DNBOX_HOST_SOURCE
#error Supply the authenticated current-source host/x0x_host.c path
#endif
#define main original_host_main
#include DNBOX_HOST_SOURCE
#undef main
static unsigned probe_checks;
#define CHECK(x) do {probe_checks++;if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void settle(void){int32_t a[512];for(unsigned i=0;i<12;i++)engine_render(a,256);}
int main(int argc,char **argv){
 CHECK(argc==5);char *end=0;long target=strtol(argv[4],&end,10);CHECK(end&&!*end&&target>=-1&&target<48);load_force=0;boot();settle();unsigned total=0;int nb=engine_brk_nslots();
 for(unsigned k=0;k<3;k++){
  memset(slot_img[k],255,sizeof slot_img[k]);slot_is_img[k]=1;usr_nz[k]=0;
  if(strcmp(argv[k+1],"-")){FILE*f=fopen(argv[k+1],"rb");CHECK(f);size_t got=fread(slot_img[k],1,sizeof slot_img[k],f);int extra=fgetc(f);CHECK(!ferror(f)&&extra==EOF);fclose(f);CHECK(got>=512);smp_user_scan(k);CHECK(usr_nz[k]>0);total+=usr_nz[k];}
 }
 engine_brk_loops();CHECK(engine_brk_nslots()==nb+(int)total);
 printf("{\"builtin_count\":%d,\"user_count\":%u,\"menu_count\":%d,\"zones\":[",nb,total,engine_brk_nslots());unsigned at=0;
 for(unsigned k=0;k<3;k++)for(unsigned z=0;z<usr_nz[k];z++){
  uint32_t n,rate;char name[9];const uint8_t*d=plat_slot(k,z,&n,&rate,name);CHECK(d&&rate==22050u);
  unsigned menu=(unsigned)nb+at;seq.pat[seq.ppat[TRK_BRK]].brk.slot_a=menu;engine_brk_loops();int rendered=(long)at==target;if(rendered)engine_brk_live(0,1);
  int32_t audio[512];unsigned nonzero=0;double energy=0;
  for(unsigned b=0;rendered&&b<16;b++){engine_render(audio,256);for(unsigned i=0;i<512;i++){if(audio[i])nonzero++;double x=(double)audio[i]/2147483648.;energy+=x*x;}}
  if(rendered){engine_brk_live(0,0);CHECK(nonzero>0&&energy>0);}
  printf("%s{\"slot\":%u,\"zone\":%u,\"frames\":%u,\"rate\":%u,\"data_crc32\":%u,\"rendered\":%s,\"engine_nonzero\":%u,\"engine_energy\":%.12g}",at?",":"",k,z,n,rate,st_crc32(d,(n+1)/2),rendered?"true":"false",nonzero,energy);at++;
 }
 puts("]}");CHECK(at==total);return 0;
}

