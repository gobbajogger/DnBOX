/* Other products are outside this SJ6 fixture. Fail immediately if entered. */
#include "original_runtime.h"
#include <stdlib.h>
int original_miami_backend_note_on(original_miami_backend *p,unsigned k,unsigned v){abort();}
int original_miami_backend_note_off(original_miami_backend *p,unsigned k){abort();}
int original_miami_backend_set_macro(original_miami_backend *p,unsigned k,float v){abort();}
int original_chicago_backend_set(original_chicago_backend *p,unsigned k,float v){abort();}
int original_chicago_backend_note(original_chicago_backend *p,unsigned k,unsigned v){abort();}
int s1_part_param(s1_dry_part *p,unsigned k,float v){abort();}
int s1_part_note(s1_dry_part *p,unsigned k,unsigned v){abort();}
