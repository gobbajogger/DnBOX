/* Exact isolated operator: branch/load operands mirror source but no unity hot cost. */
void typed_sine_profile_prepare(float gain[2],const unsigned char oscillator[2],const float wave[2],unsigned typed)
{
    if(typed)for(unsigned l=0;l<2;l++)
        if(oscillator[l]==2u&&wave[l]==0.f&&gain[l]<=.808f)
            gain[l]*=0x1.3cdfdep+0f;
}
float typed_sine_hot_receiver(float computed_carrier,float prepared)
{
    if(prepared!=1.f)computed_carrier*=prepared;
    return computed_carrier;
}
