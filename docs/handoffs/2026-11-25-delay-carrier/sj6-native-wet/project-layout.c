#include <stdio.h>
#include <stddef.h>
#include "app/x0x.h"
int main(void) {
printf("%zu %zu %zu %zu %zu %zu %zu %zu %zu %zu\n",sizeof(project_t), sizeof(settings_t), offsetof(settings_t,bt), offsetof(settings_t,snd_rev),offsetof(project_t,sound),offsetof(project_t,pat), offsetof(sound_t,v[T_303][1]), offsetof(sound_t,v[T_303][8]), offsetof(sound_t,v[T_303][10]),offsetof(sound_t,bass[1]));
}
