#include OTA_HEADER
extern "C" void j6_ota_init(DunneCore::OriginalOTAFilter*p,float sr){p->init(sr);}
extern "C" void j6_ota_params(DunneCore::OriginalOTAFilter*p,double cutoff,double resonance,unsigned frames){p->setParameters(cutoff,resonance,frames);}
extern "C" float j6_ota_process(DunneCore::OriginalOTAFilter*p,float input){return p->process(input);}
extern "C" __attribute__((used,section(".j6layout"))) const unsigned j6_layout[]={sizeof(DunneCore::OriginalOTAFilter),alignof(DunneCore::OriginalOTAFilter),sizeof(double),sizeof(float)};
