/* SPDX-License-Identifier: GPL-3.0-only */
/* Historical slot-2 cells and 0x4F445231 ids load as Synth One init. */
#include "../../firmware/src/app/original_sound.h"
#include "../../firmware/src/app/bass_identity.h"
#include "../../firmware/src/app/bass_pattern_preset.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const bass_identity_entry_t registry[] = {
    {ORIGINAL_SOUND_CHICAGO_ID, ORIGINAL_SOUND_SCHEMA, 22, 1},
    {ORIGINAL_SOUND_MIAMI_ID, ORIGINAL_SOUND_SCHEMA, 18, 1},
    {ORIGINAL_SOUND_SYNTHONE_ID, ORIGINAL_SOUND_SCHEMA, ORIGINAL_SYNTHONE_NPARAMS, 1},
    {ORIGINAL_SOUND_JUNO_ID, ORIGINAL_SOUND_SCHEMA, ORIGINAL_JUNO_NPARAMS, 1}
};

int main(void)
{
    uint8_t cell[ORIGINAL_SOUND_BYTES] = {0};
    unsigned i, kind, one;
    /* Historical Cathedral 8-parameter cell, kind 2, factory 0. */
    static const uint8_t hist[6] = {57,66,51,23,83,95};
    memcpy(cell, hist, 6);
    cell[26] = 'O'; cell[27] = 'S'; cell[28] = ORIGINAL_SOUND_SCHEMA;
    cell[29] = 2; cell[30] = 0;
    assert(original_sound_is_legacy_slot2_cell(cell));
    assert(original_sound_adopt_legacy_cell(cell) == 1);
    assert(original_sound_valid(cell));
    assert(cell[29] == ORIGINAL_SOUND_SYNTHONE && cell[30] == 0);
    assert(original_sound_engine_id(cell[29]) == ORIGINAL_SOUND_SYNTHONE_ID);
    assert(!strcmp(original_sound_name(ORIGINAL_SOUND_SYNTHONE, 0), "INIT"));
    for (i = 1; i < 22; ++i) assert(cell[i] == 0);
    assert(!original_sound_is_legacy_slot2_cell(cell));
    assert(original_sound_adopt_legacy_cell(cell) == 0);

    /* Historical factory 3 (0-based) also becomes init. */
    memset(cell, 0, sizeof cell);
    memcpy(cell, hist, 6);
    cell[26] = 'O'; cell[27] = 'S'; cell[28] = ORIGINAL_SOUND_SCHEMA;
    cell[29] = 2; cell[30] = 3;
    assert(original_sound_adopt_legacy_cell(cell) == 1);
    assert(cell[30] == 0);

    bass_identity_t tag = {
        ORIGINAL_SOUND_LEGACY_ODR1_ID, ORIGINAL_SOUND_SCHEMA, 8, BASS_IDENTITY_KNOWN
    };
    assert(bass_identity_adopt(&tag) == 1);
    assert(tag.engine == ORIGINAL_SOUND_SYNTHONE_ID);
    assert(tag.count == ORIGINAL_SYNTHONE_NPARAMS);
    assert(bass_identity_check(&tag, registry, 4) == BASS_IDENTITY_READY);

    uint8_t wire[8] = {0};
    wire[0] = (uint8_t)ORIGINAL_SOUND_LEGACY_ODR1_ID;
    wire[1] = (uint8_t)(ORIGINAL_SOUND_LEGACY_ODR1_ID >> 8);
    wire[2] = (uint8_t)(ORIGINAL_SOUND_LEGACY_ODR1_ID >> 16);
    wire[3] = (uint8_t)(ORIGINAL_SOUND_LEGACY_ODR1_ID >> 24);
    wire[4] = ORIGINAL_SOUND_SCHEMA;
    wire[6] = 8;
    wire[7] = BASS_IDENTITY_KNOWN;
    assert(bass_identity_decode(&tag, wire, 8, registry, 4) == BASS_IDENTITY_READY);
    assert(tag.engine == ORIGINAL_SOUND_SYNTHONE_ID);
    /* New writes use OS11, never the historical id. */
    assert(bass_identity_encode(wire, 8, &tag, registry, 4) == BASS_IDENTITY_READY);
    uint32_t written = (uint32_t)wire[0] | (uint32_t)wire[1] << 8 |
        (uint32_t)wire[2] << 16 | (uint32_t)wire[3] << 24;
    assert(written == ORIGINAL_SOUND_SYNTHONE_ID);

    assert(bass_pattern_preset_decode(0xC1u, &kind, &one) && kind == ORIGINAL_SOUND_SYNTHONE && one == 1);
    assert(bass_pattern_preset_decode(0xC4u, &kind, &one) && kind == ORIGINAL_SOUND_SYNTHONE && one == 1);
    assert(bass_pattern_preset_encode(ORIGINAL_SOUND_SYNTHONE, 1) == 0xC1u);
    assert(!bass_pattern_preset_encode(ORIGINAL_SOUND_SYNTHONE, 2));
    printf("legacy-slot2-map: PASS\n");
    return 0;
}
