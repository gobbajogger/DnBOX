#include <stddef.h>
#include <stdint.h>
#include "drum909.h"
#include "drum808.h"
__attribute__((used,section(".traffic_layout"))) const uint32_t offsets[]={
 sizeof(drum909_t),offsetof(drum909_t,send_rev),offsetof(drum909_t,send_dly),DR_NUM,
 sizeof(drum808_t),offsetof(drum808_t,potv),D8S_NUM,D8P_NUM,D8P_REV,D8P_DLY
};
