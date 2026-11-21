/* SPDX-License-Identifier: GPL-3.0-only */
/* Render the three recovery paths using the register record observed on target. */
#define main archived_fixture_main
#include "native_routes.c"
#undef main

int main(void)
{
    FILE *input = fopen("crash.bin", "rb");
    assert(input && fread(&fm1_crash, 1, sizeof fm1_crash, input) == sizeof fm1_crash);
    assert(fgetc(input) == EOF && fclose(input) == 0);
    assert(fm1_crash.magic == FM1_CRASH_MAGIC);

    lcd_init();
    palette_set(1);
    strcpy(exit_frame, "fault");
    if (!setjmp(reset_destination)) fm1_fault(&fm1_crash);

    scenario = SAFE_PLAY;
    strcpy(entry_frame, "safe");
    strcpy(exit_frame, "retry");
    if (!setjmp(reset_destination)) safe_main();

    scenario = START_NORMAL;
    safe_mode = 0;
    strcpy(exit_frame, "post-crash");
    if (!setjmp(reset_destination)) fm1_main();
    return 0;
}
