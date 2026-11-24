/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UI_PERF_PROFILE_H
#define UI_PERF_PROFILE_H

/* Stock SMK keeps the overload guard and normal About readout, not the
 * synthetic PERF benchmark or the detailed stage instrumentation it uses. */
#ifndef X0X_UI_PERF
#if BOARD_SMK37 && !defined(X0X_HOST) && !(defined(X0X_LCD_TRACE) && X0X_LCD_TRACE)
#define X0X_UI_PERF 0
#else
#define X0X_UI_PERF 1
#endif
#endif
#ifndef X0X_ENGINE_PROFILE
#if X0X_UI_PERF || defined(X0X_HOST)
#define X0X_ENGINE_PROFILE 1
#else
#define X0X_ENGINE_PROFILE 0
#endif
#endif

#endif
