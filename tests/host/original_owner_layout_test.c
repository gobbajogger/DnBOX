/* SPDX-License-Identifier: GPL-3.0-only */
#include "../../firmware/src/app/original_part_owner.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
int main(void)
{
    original_part_owner parts[2];
    assert(offsetof(original_part_owner, runtime) == 0);
    assert(offsetof(original_part_owner, fx) == sizeof(original_runtime_part));
    assert(sizeof parts == 2 * (sizeof(original_runtime_part) + sizeof(original_chicago_fx2_compact)));
    assert((uintptr_t)&parts[0].fx + sizeof parts[0].fx <= (uintptr_t)&parts[1]);
    assert((uintptr_t)&parts[0].runtime != (uintptr_t)&parts[1].runtime);
    assert((uintptr_t)&parts[0].fx != (uintptr_t)&parts[1].fx);
    assert((uintptr_t)&parts[0].fx % _Alignof(original_chicago_fx2_compact) == 0);
    assert((uintptr_t)&parts[1].fx % _Alignof(original_chicago_fx2_compact) == 0);
    printf("PASS owner layout: total=%zu runtime=%zu fx=%zu alignment=%zu; disjoint parts\n",
           sizeof(original_part_owner), sizeof(original_runtime_part),
           sizeof(original_chicago_fx2_compact), _Alignof(original_part_owner));
    return 0;
}
