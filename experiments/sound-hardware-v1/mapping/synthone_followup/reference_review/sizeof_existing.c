#include "chicago_mp4.h"
#include "miami_ms20.h"
#include "../../../sound_lane/firmware/src/dsp/master.h"
#include <stdio.h>
int main(void) {
    printf("{\"scope\":\"host type layouts only, not linked firmware cost\","
           "\"chicago_mp4_state\":%zu,\"miami_ms20_state\":%zu,"
           "\"d9_biquad_declared_nine_float_storage\":%zu,\"master_whole_state\":%zu,"
           "\"chicago_tables_uncompressed_two_arrays_bytes\":%u,"
           "\"miami_filter_tables_uncompressed_four_arrays_bytes\":%u}\n",
           sizeof(original_chicago_mp4),sizeof(original_miami_ms20),
           9u*sizeof(float),sizeof(master_t),2u*16385u*4u,4u*129u*4u);
}
