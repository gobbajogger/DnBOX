/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include "original_runtime.h"
#include "../../../experiments/original_synth/chicago_fx2_compact.h"

/* Stable, exclusively owned storage for one runtime and its borrowed Chicago FX.
 * Never copy/move a live object. A replacement must successfully retire the
 * runtime before repurposing either member; callbacks point into this object. */
typedef struct {
    original_runtime_part runtime;
    original_chicago_fx2_compact fx;
} original_part_owner;
_Static_assert(sizeof(original_part_owner) == sizeof(original_runtime_part) +
               sizeof(original_chicago_fx2_compact), "part grouping adds no RAM");
