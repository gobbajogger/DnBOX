/* SPDX-License-Identifier: GPL-3.0-only */
#include "../../firmware/src/app/original_runtime.h"
#include <stdio.h>
int main(void){
 /* Two runtime parts are resident in .pool. This is a host ABI
  * regression sentinel, NOT a substitute for the target linker check. */
 const size_t max_part_bytes = 5368u;
 /* Keep the unchanged non-arp storage ceiling, plus a separate checked
  * budget for the later shared live arp (160 bytes at this host ABI).
  * Native builds still enforce the full target .pool margin. */
 size_t actual = sizeof(original_runtime_part);
#if X0X_ARP_ENGINE
 const size_t arp_bytes = sizeof(original_runtime_arp);
 if(arp_bytes > 160u){
  fprintf(stderr,"FAIL shared arp runtime sizeof %zu > 160\n",arp_bytes);
  return 1;
 }
 actual -= arp_bytes;
 printf("PASS shared arp runtime sizeof %zu <= 160\n",arp_bytes);
#endif
 if(actual > max_part_bytes){
  fprintf(stderr,"FAIL Miami runtime-part sizeof %zu > %zu\n",actual,max_part_bytes);
  return 1;
 }
 printf("PASS Miami runtime-part sizeof %zu <= %zu\n",actual,max_part_bytes);
 return 0;
}
