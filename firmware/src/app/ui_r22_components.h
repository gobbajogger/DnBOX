/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
/* Shared logical geometry. Small rectangles clamp radius to half their bounds
 * in rbox/rframe; that is clipping, never a second corner-style token. */
enum { R22_RADIUS=9,R22_GUTTER=4,R22_GRID_X=6,R22_GRID_W=228,
       R22_COLUMNS=4,R22_COLUMN_W=57,R22_PANEL_X=3,R22_PANEL_W=234,
       R22_CONTROL_H=72,R22_COMPACT_H=42 };
static int r22_cell_center(int cell,int count)
{ return R22_GRID_X+(R22_GRID_W*(2*cell+1))/(2*count); }
