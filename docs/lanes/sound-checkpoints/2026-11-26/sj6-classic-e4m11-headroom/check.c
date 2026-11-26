#include "history.h"
uint16_t enc(unsigned i){return sj6_hist_encode(sj6_word_float(i));}
unsigned dec(uint16_t i){return sj6_float_word(sj6_hist_decode(i));}
