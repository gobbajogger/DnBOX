/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBX_SJ6_PRESET_FIELDS_H
#define DNBX_SJ6_PRESET_FIELDS_H
#include "dnbx_core.h"

typedef struct { unsigned id; float value; } sj6_preset_field;
enum sj6_layer_admission { SJ6_LINKS_UNKNOWN, SJ6_INDEPENDENT_ENVELOPES };
enum { SJ6_FIELD_STORED = 1, SJ6_E_UNQUALIFIED_LINK = -5 };

/* A caller serializes all access to the part and supplies immutable ordered
 * fields. ID36 binding requires an independently configured envelope owner.
 * Link values 115 and 116 are checked across the batch and retained data.
 * Success is DNBX_OK if attack was applied or SJ6_FIELD_STORED for raw-only
 * batches. Invalid or unsupported attack batches leave the part unchanged. */
int sj6_apply_preset_fields(dnbx_part *part, enum sj6_layer_admission admission,
                            const sj6_preset_field *fields, size_t count);
#endif
