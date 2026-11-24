static void osc1_project(unsigned owner,unsigned scenario,int export)
{
    const char *dir=getenv("OSC1_WIRE_PATH"); CHECK(dir);
    char name[1024];snprintf(name,sizeof name,"%s/both-%u-%u",dir,scenario,owner);
    FILE *f=fopen(name,export ? "wb" : "rb"); CHECK(f);
    const unsigned objects[]={OBJ_SET,OBJ_SOUND,OBJ_PAT0,OBJ_PAT1,OBJ_SONG,OBJ_MOTION};
    uint8_t data[PLAT_STORE_MAX];
    for(unsigned i=0;i<sizeof objects/sizeof *objects;++i){
        int n;
        if(export){n=plat_store_load(objects[i],data,sizeof data);CHECK(n>=0);CHECK(fwrite(&n,sizeof n,1,f)==1);CHECK(fwrite(data,1,(size_t)n,f)==(size_t)n);}
        else {CHECK(fread(&n,sizeof n,1,f)==1&&n>=0&&(unsigned)n<=sizeof data);CHECK(fread(data,1,(size_t)n,f)==(size_t)n);CHECK(!plat_store_save(objects[i],data,(size_t)n));}
    }
    if(!export) CHECK(fgetc(f)==EOF);
    CHECK(!fclose(f));
}
