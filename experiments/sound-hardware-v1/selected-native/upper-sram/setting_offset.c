#include <stddef.h>
#include <stdio.h>
#include "x0x.h"
int main(void) { printf("%zu %zu\n", offsetof(settings_t, bt), sizeof(settings_t)); }
