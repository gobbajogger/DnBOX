/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

enum { S1_FACTORY_COMMANDS = 144 };
/* Name and stored program position come from the original bank. */
int s1_factory_lookup(const char *bank, unsigned position);
const char *s1_factory_name(unsigned program);
int s1_factory_read(unsigned program, unsigned command, unsigned *parameter, float *physical);

#ifdef __cplusplus
}
#endif
