/* SPDX-License-Identifier: GPL-3.0-only */
#include "sj6_preset_fields.h"
#include <string.h>

enum { LAYER1_ATTACK = 36, ADSR_LINK = 115, ADSR_FILTER_LINK = 116 };

/* The raw core accepts every finite binary32 value. Match that admission
 * for the whole batch before changing either the physical or raw owner. */
static int finite_word(float value)
{
    uint32_t word;
    memcpy(&word, &value, sizeof word);
    return (word & UINT32_C(0x7f800000)) != UINT32_C(0x7f800000);
}

int sj6_apply_preset_fields(dnbx_part *part, enum sj6_layer_admission admission,
                            const sj6_preset_field *fields, size_t count)
{
    dnbx_layer_params layer;
    int status = dnbx_get_layer(part, 0, &layer);
    if (status != DNBX_OK) return status;
    if (count && !fields) return DNBX_E_ARGUMENT;

    unsigned written_links = 0;
    int ambiguous_link = 0;
    int has_attack = 0;
    for (size_t i = 0; i < count; ++i) {
        const sj6_preset_field field = fields[i];
        if (field.id >= DNBX_RAW_CONTROLS || !finite_word(field.value)) return DNBX_E_RANGE;
        if (field.id == LAYER1_ATTACK) {
            status = dnbx_attack_from_unit(field.value, &layer.amp.attack_s);
            if (status != DNBX_OK) return status;
            has_attack = 1;
        } else if (field.id == ADSR_LINK || field.id == ADSR_FILTER_LINK) {
            written_links |= 1u << (field.id - ADSR_LINK);
            if (field.value != 0.0f) ambiguous_link = 1;
        }
    }

    if (has_attack) {
        /* Preset order places ID36 before both link flags. Check the full
         * batch first. An omitted field inherits the part's recorded raw
         * value. Absent flags need the caller's explicit independent mode. */
        if (admission != SJ6_INDEPENDENT_ENVELOPES || ambiguous_link)
            return SJ6_E_UNQUALIFIED_LINK;
        for (unsigned id = ADSR_LINK; id <= ADSR_FILTER_LINK; ++id) {
            float retained;
            if (!(written_links & (1u << (id - ADSR_LINK))) &&
                dnbx_read_raw(part, id, &retained) == DNBX_OK && retained != 0.0f)
                return SJ6_E_UNQUALIFIED_LINK;
        }
        status = dnbx_set_layer(part, 0, &layer);
        if (status != DNBX_OK) return status;
    }

    /* All raw arguments were validated, the part is initialized, and the
     * caller holds exclusive access throughout this transaction. */
    for (size_t i = 0; i < count; ++i)
        (void)dnbx_store_raw(part, fields[i].id, fields[i].value);
    return has_attack ? DNBX_OK : SJ6_FIELD_STORED;
}
