/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef SMK_INLOG_PROFILE_H
#define SMK_INLOG_PROFILE_H

/* A native trace build keeps command 38 and its recorder. Stock firmware
 * leaves both out; the recorder has no effect on MIDI handling. */
#ifndef X0X_SMK_INLOG
#define X0X_SMK_INLOG (BOARD_SMK37 && X0X_LCD_TRACE)
#endif
#if X0X_SMK_INLOG
#include <stdint.h>
void smk_inlog(uint32_t src, uint32_t pkt);
#define SMK_INLOG_RECORD(src, pkt) smk_inlog((src), (pkt))
#else
#define SMK_INLOG_RECORD(src, pkt) ((void)0)
#endif

#endif
