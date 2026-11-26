#ifndef FN
#error measured symbol required
#endif
void FN(const float in[512][6],float out[512][6],unsigned active) {
  OTA owner[6];
  memset(owner,0,sizeof(owner));
  static const float params[4]={.17f,.33f,.93f,.15f};
  static const float coeff[5]={.183f,.73f,.329f,.88f,.34f};
#ifdef TANGENT
  float tile[5];
  prepare_tile(params,tile);
#endif
  for(unsigned frame=0;frame<512;frame++) {
    for(unsigned v=0;v<active;v++) {
#ifdef TANGENT
      out[frame][v] = physical_tick(&owner[v],in[frame][v],tile,coeff);
#else
      out[frame][v] = physical_tick(&owner[v],in[frame][v],params,coeff);
#endif
    }
  }
}
