/* Check exact old dispatch in host, with virtual transport and no sample access.
 * The stage already contains a complete CRC-correct upload, not a physical transfer. */
#ifndef SMP_USER_SLOTS
#define SMP_USER_SLOTS PLAT_NSLOTS
#endif
#define ED_READ_MAX 224u
#include "old-ed-project-generated.inc"

static void old_usb_probe(void)
{
 uint8_t begin[]={67,PROJECT_FILE_SIZE&127u,(PROJECT_FILE_SIZE>>7)&127u,(PROJECT_FILE_SIZE>>14)&127u};
 const uint8_t validate[]={74},commit[]={69,0x58,0x30},abort_packet[]={70};
 check=proj; uint32_t start=store_writes;
 ed_project_handle(begin,sizeof begin);
 CHECK(usb_reply==0 && project_busy && ed_project.mode==2 && !ed_project.valid);
 CHECK(ed_project_samples_locked());
 project_stage=fresh;
 wire_header(ed_project.header,&project_stage);
 ed_project.next=ED_PROJECT_SIZE;
 ed_project_handle(validate,sizeof validate);
 CHECK(usb_reply==5 && !ed_project.valid && ed_project_samples_locked());
 ed_project_handle(commit,sizeof commit);
 CHECK(usb_reply==5 && !ed_project.valid && project_busy && ed_project_samples_locked());
 ed_project_handle(abort_packet,sizeof abort_packet);
 CHECK(usb_reply==0 && !ed_project.mode && !project_busy);
 CHECK(!memcmp(&check,&proj,sizeof proj) && store_writes==start && !sample_effects);
 puts("PASS exact ED_PROJECT BEGIN/VALIDATE/COMMIT/ABORT, no stage adoption, samples locked and zero writes on legacy reader");
}
