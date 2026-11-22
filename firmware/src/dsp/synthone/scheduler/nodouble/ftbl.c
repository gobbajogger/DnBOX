#include "soundpipe.h"
/* Integer log2 of an integer ratio. For the power-of-two table sizes this is
 * the same value as trunc(log2(SP_FT_MAXLEN / size)). sicvt uses binary32
 * division, which matches the double division rounded back to binary32 at
 * 44100, 48000 and 96000. */
int sp_ftbl_init(sp_data *sp, sp_ftbl *ft, size_t size)
{
    size_t n;
    int lobits = 0;
    ft->size = size;
    ft->sicvt = (float)SP_FT_MAXLEN / sp->sr;
    n = size ? SP_FT_MAXLEN / size : 0;
    while (n > 1) { n >>= 1; lobits++; }
    ft->lobits = lobits;
    ft->lomask = (1 << ft->lobits) - 1;
    ft->lodiv = 1.f / (float)(1 << ft->lobits);
    ft->del = 1;
    return SP_OK;
}
