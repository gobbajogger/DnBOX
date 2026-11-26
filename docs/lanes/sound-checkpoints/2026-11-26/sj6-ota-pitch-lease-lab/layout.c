#include <stddef.h>
#include "owner.h"
#define MARK(n,z) const char n[(z)+1]={0}
MARK(voice_stride,sizeof(dnbx_voice));
MARK(layer_voice_stride,sizeof(dnbx_layer_voice));
MARK(pitch_owner_off, offsetof(dnbx_voice,sj6_pitch));
MARK(pitch_owner_size,sizeof(((dnbx_voice*)0)->sj6_pitch));
MARK(velocity_off,offsetof(dnbx_voice,velocity));
MARK(part_size,sizeof(dnbx_part));
MARK(part_voices,offsetof(dnbx_part,voice));
#if DNBX_DIAGNOSTIC_SJ6_PITCH_LEASE
_Static_assert(offsetof(dnbx_voice,sj6_pitch)==offsetof(dnbx_voice,ota_main_lease),"shared alignment");
_Static_assert(sizeof(((dnbx_voice*)0)->ota_main_lease)==2*sizeof(float),"only two Float32 words");
#endif
