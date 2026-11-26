#include <stdio.h>
#define HP_PROBE_TICKS() (0u)
#include "pin-bench.h"
int main(void){
 sj6_hp_probe();
 printf("reference=%08x n=%u\n",sj6_hp_probe_result.digest,sj6_hp_probe_result.count);
}
