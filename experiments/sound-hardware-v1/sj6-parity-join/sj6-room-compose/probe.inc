/* Private fixture selects real runtime downstream call, not product API.
 * Sources already crossed the explicitly named boundaries. */
__attribute__((visibility("default"))) int compose_probe(RoomOwner *x,const RoomBlock *b,float *l,float *r,unsigned n,float master){
 if(!(master>=0.f&&master<=1.f)||!compose_room_admits(x,b,l,r,n))return 0;
 original_runtime_part p={0};
 p.kind=ORIGINAL_SOUND_JUNO;p.status=ORIGINAL_RUNTIME_OK;
 p.dsp.juno.chorus.factory_mode=JUNO6_CHORUS_OFF;
 p.dsp.juno.synth.master_gain=master;
 p.lab_room=x;p.lab_room_block=b;
 sj6_run_effects(&p,l,r,n);
 return 1;
}
__attribute__((visibility("default"))) int compose_probe_disabled(float*l,float*r,unsigned n){
 original_runtime_part p={0};
 p.kind=ORIGINAL_SOUND_JUNO;p.status=ORIGINAL_RUNTIME_OK;
 p.dsp.juno.chorus.factory_mode=JUNO6_CHORUS_OFF;
 p.dsp.juno.synth.master_gain=1.f;
 sj6_run_effects(&p,l,r,n);
 return 1;
}
