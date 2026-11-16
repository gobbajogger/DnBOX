/* Proves s1_midi_cps_bits[n] equals (float)(pow(2,(n-69)/12)*440) for n = 0..127. */
#include "midi_cps.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    int bad = 0;
    for (int n = 0; n < 128; n++) {
        float ref = (float)(pow(2.0, (n - 69.0) / 12.0) * 440.0);
        float got = s1_midi_cps_at(n);
        if (memcmp(&got, &ref, sizeof got)) {
            bad++;
            fprintf(stderr, "note %d table %a ref %a\n", n, got, ref);
        }
    }
    printf("{\"notes\":128,\"mismatches\":%d}\n", bad);
    return bad ? 1 : 0;
}
