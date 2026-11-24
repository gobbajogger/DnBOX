#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"
#include "osc1_old_wire.h"
int main(void)
{
    for(unsigned v=0;v<NBASS;++v) for(unsigned form=0;form<2;++form) {
        setup(V_PART,PART_303A+v);
        osc1_project(v,form,0);
        const unsigned writes=store_writes;
        CHECK(project_load()==-9 && project_storage_unsupported());
        CHECK(project_save()==-9 && store_writes==writes);
    }
    printf("PASS previous reader rejects marked OSC1 Sound plus lock-only before any writes (%u checks)\n",checks);
    return 0;
}
