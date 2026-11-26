from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a='static void rv8(s1c *s,float il,float ir,float *ol,float *orr){'
assert s.count(a)==1
s=s.replace(a,'''/* Observation-only single room instance, this does not qualify arena usage. */
static int16_t rv8_extra_decode[8][32];
static unsigned rv8_extra_write;
static FILE* rv8_original_trajectory;
static int rv8_trajectory_enabled=-1;
'''+a+'''
 float tau[8];
 if(rv8_trajectory_enabled<0){
    rv8_trajectory_enabled=0;
    const char*m=getenv("ROOM_HISTORY");
    if(m && !strcmp(m,"modulated")){
       rv8_trajectory_enabled=1;
       rv8_original_trajectory=fopen(getenv("ROOM_TRAJECTORY"),"rb");
       if(!rv8_original_trajectory)abort();
    }
 }
 if(rv8_trajectory_enabled){
    for(unsigned m=0;m<7;++m)
       if(fread(tau,sizeof(float),8,rv8_original_trajectory)!=8)abort();
 }
''')
a='  float x=q->pred[0][i]*(1.f/4096.f);'
assert s.count(a)==1
s=s.replace(a,'''  rv8_extra_decode[i][rv8_extra_write]=q->pred[0][i];
  const float h=rv8_trajectory_enabled?tau[i]*(1.f/7.f)-(float)rv8_lengths[i]+16.f:16.f;
  if(h<0.f||h>=31.f)abort();
  const unsigned lag=(unsigned)h;
  const float u=h-(float)lag;
  const float early=rv8_extra_decode[i][(rv8_extra_write-lag)&31u];
  const float late=rv8_extra_decode[i][(rv8_extra_write-lag-1u)&31u];
  float x=(early+(late-early)*u)*(1.f/4096.f);''')
assert s.count('w+1u==rv8_lengths[i]?')==1
s=s.replace('w+1u==rv8_lengths[i]?','w+1u==rv8_lengths[i]-16u?')
s=s.replace(' *ol=.35f*l;*orr=.35f*r;',' rv8_extra_write=(rv8_extra_write+1u)&31u;\n *ol=.35f*l;*orr=.35f*r;')
p.write_text(s)
