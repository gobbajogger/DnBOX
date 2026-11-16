/* SPDX-License-Identifier: GPL-3.0-only */
#include "synthone_c.h"
#include "source_scheduler.hpp"
#include <new>
#include <stdint.h>
struct dnbox_s1 { s1scheduler::SourceScheduler engine; };
#if defined(__PI32V2__)
static_assert(sizeof(dnbox_s1)==DNBOX_S1_PI32V2_OWNER_BYTES,"review owner allocation after layout change");
static_assert(alignof(dnbox_s1)==DNBOX_S1_PI32V2_OWNER_ALIGNMENT,"review owner alignment after layout change");
#endif
static bool span(const void *p,size_t n) {
    return p && n <= UINTPTR_MAX-reinterpret_cast<uintptr_t>(p);
}
static bool disjoint(const void *a,size_t n,const void *b,size_t m) {
    return span(a,n) && span(b,m) &&
        (reinterpret_cast<uintptr_t>(a)+n <= reinterpret_cast<uintptr_t>(b) ||
         reinterpret_cast<uintptr_t>(b)+m <= reinterpret_cast<uintptr_t>(a));
}
extern "C" {
size_t dnbox_s1_owner_bytes(void) { return sizeof(dnbox_s1); }
size_t dnbox_s1_owner_alignment(void) { return alignof(dnbox_s1); }
dnbox_s1 *dnbox_s1_init(void *storage,size_t bytes,const dnbox_s1_config *config) {
    if(bytes<sizeof(dnbox_s1) || !span(storage,bytes) ||
       reinterpret_cast<uintptr_t>(storage)%alignof(dnbox_s1) ||
       reinterpret_cast<uintptr_t>(config)%alignof(dnbox_s1_config) ||
       !disjoint(storage,sizeof(dnbox_s1),config,sizeof(*config))) return nullptr;
    const dnbox_s1_config c=*config;
    if(c.sine_count>SIZE_MAX/sizeof(float) ||
       !disjoint(storage,sizeof(dnbox_s1),c.compact,c.compact_bytes) ||
       !disjoint(storage,sizeof(dnbox_s1),c.sine,c.sine_count*sizeof(float)) ||
       !disjoint(storage,sizeof(dnbox_s1),c.frequency_bands,13*sizeof(float)) ||
       !disjoint(storage,sizeof(dnbox_s1),c.parameters,150*sizeof(float)) ||
       !disjoint(storage,sizeof(dnbox_s1),c.tuning,128*sizeof(float)) ||
       reinterpret_cast<uintptr_t>(c.frequency_bands)%alignof(float)) return nullptr;
    synthone_shared::Tables tables{};
    for(unsigned b=0;b<13;b++) {
        tables.frequency_band[b]=c.frequency_bands[b];
        for(unsigned s=0;s<4;s++) tables.morph[b*4+s]={nullptr,4097,b,static_cast<synthone_shared::Shape>(s)};
    }
    tables.sine=c.sine;tables.sine_count=c.sine_count;
    dnbox_s1 *owner=new(storage) dnbox_s1;
    if(!owner->engine.init_compact_metadata(tables,c.compact,c.compact_bytes,c.sample_rate,
                                          c.parameters,150,c.tuning,128)) {
        owner->~dnbox_s1();return nullptr;
    }
    return owner;
}
void dnbox_s1_retire(dnbox_s1 *p) { if(p) p->~dnbox_s1(); }
int dnbox_s1_parameter(dnbox_s1 *p,unsigned id,float value) { return p && p->engine.set_target(id,value); }
int dnbox_s1_note_on(dnbox_s1 *p,int note,int velocity) { return p && p->engine.start(note,velocity); }
int dnbox_s1_note_off(dnbox_s1 *p,int note) { return p && p->engine.stop(note); }
int dnbox_s1_all_off(dnbox_s1 *p) { return p && p->engine.stop_all(); }
int dnbox_s1_reset(dnbox_s1 *p) { return p && p->engine.reset(); }
int dnbox_s1_render(dnbox_s1 *p,unsigned n,float *l,float *r) { return p && p->engine.process(n,0,l,r); }
}
