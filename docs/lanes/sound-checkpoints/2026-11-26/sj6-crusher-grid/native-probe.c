#include "converter.h"
int probe(sj6_crush_applied*s,unsigned id,unsigned q,float raw){return sj6_crush_commit(s,id,q,raw);}
