/* Comparison-only impulse capture using the public filter interface. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "soundpipe.h"
int main(int argc, char **argv) {
    if (argc != 4) return 2;
    float hz = strtof(argv[1], NULL), resonance = strtof(argv[2], NULL);
    if (!(hz >= 64 && hz <= 22050 && resonance >= 0 && resonance <= .98f)) return 2;
    FILE *out = fopen(argv[3], "wbx");
    if (!out) return 2;
    sp_data context = {0};
    context.sr = 44100;
    sp_moogladder *filter = NULL;
    if (sp_moogladder_create(&filter) != SP_OK || sp_moogladder_init(&context, filter) != SP_OK) return 1;
    filter->freq = hz;
    filter->res = resonance;
    for (int i=-4096; i<32768; ++i) {
        float input = i == 0 ? .0001f : 0.0f, value = 0.0f;
        if (sp_moogladder_compute(&context, filter, &input, &value) != SP_OK || !isfinite(value)) return 1;
        if (i >= 0 && fwrite(&value, sizeof(value), 1, out) != 1) return 1;
    }
    sp_moogladder_destroy(&filter);
    return fclose(out) != 0;
}
