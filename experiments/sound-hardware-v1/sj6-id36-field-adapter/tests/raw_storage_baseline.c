/* SPDX-License-Identifier: GPL-3.0-only */
/* Frozen raw storage path, exposed through the candidate test signature. */
#include "sj6_preset_fields.h"
int sj6_apply_preset_fields(dnbx_part *part, enum sj6_layer_admission admission,
                            const sj6_preset_field *fields, size_t count)
{
    (void)admission;
    for (size_t i = 0; i < count; ++i) {
        int status = dnbx_store_raw(part, fields[i].id, fields[i].value);
        if (status < 0) return status;
    }
    return SJ6_FIELD_STORED;
}
