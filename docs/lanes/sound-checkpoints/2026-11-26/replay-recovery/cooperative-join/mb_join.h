/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBX_MB_JOIN_H
#define DNBX_MB_JOIN_H
#include "join.h"
int mb_decline(const join_word *cancel,join_word *declined,uint32_t generation);
enum join_result mb_poll(const join_word *done,const join_word *declined,uint32_t generation);
#endif
