#pragma once
enum { CLASSIC_I_DELAY_FRAMES=1058 };
typedef struct {
    float ring[2][CLASSIC_I_DELAY_FRAMES];
    unsigned write[2];
    float phase;
} classic_i;
void classic_i_frame(classic_i *,float l,float r,float depth,
    float phase_increment,const float feedback[2],float mix,float out[2]);
