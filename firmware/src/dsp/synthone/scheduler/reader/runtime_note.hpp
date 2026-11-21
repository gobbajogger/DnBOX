#pragma once
#include "portable_interface.hpp"
#include "direct_reader.h"
namespace s1scheduler_reader {
bool run_note(S1NoteState &,int,float *,float *,const s1_morph_binding &);
}
