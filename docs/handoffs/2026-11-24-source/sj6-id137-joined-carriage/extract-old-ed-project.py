"""Extract unmodified ED_PROJECT dispatch/data and patch no product code."""
from pathlib import Path
import sys
source,out=map(Path,sys.argv[1:])
text=source.read_text()
start=text.index('static struct {\n    uint32_t at, next;')
stop=text.index('static uint32_t ed_project_sample_len',start)
block=text[start:stop]
a=text.index('static uint32_t ed_u21(',stop)
b=text.index('static uint32_t ed_smp_slot(',a)
out.write_text(block + '''
static uint32_t fm1_ms, usb_reply;
static uint8_t ed_smp_buf[ED_READ_MAX];
static int sample_effects;
static void ed_project_sample_snapshot(void) { CHECK(!sample_effects); }
static int ed_project_samples_restored(void) { return !ed_project.changed && !sample_effects; }
static void ed_smp_disarm_unused(void) { CHECK(!sample_effects); }
static void ed_b(uint32_t n) { usb_reply=n; }
static void ed_u(uint32_t n,int bytes) { (void)n;(void)bytes; CHECK(0); }
static uint32_t ed_g(const uint8_t *p,int n) {
 uint32_t r=0;for(int i=0;i<n;i++)r|=(uint32_t)p[i]<<(7*i);return r;
}
static uint32_t ed_unpack_exact(const uint8_t *a,uint32_t n,uint8_t *out,uint32_t max) {
 (void)a;(void)n;(void)out;(void)max;CHECK(0);return 0;
}
static void ed_put7(const uint8_t *p,uint32_t n) { (void)p;(void)n;CHECK(0); }
''' + text[a:b])
