/* SPDX-License-Identifier: GPL-3.0-only */
#include "dnpool.h"
#define MAGIC 0x33444244u /* DBD3 */
#define HDR 16u
#define ENT 17u
#define MAXBODY (2u+18u*DNP_DATA)
#define ALL ((1u<<DNP_DATA)-1u)
typedef struct {const uint8_t *body;uint32_t gen,used;unsigned length,slot;} directory;
static uint32_t le(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void put(uint8_t *p,uint32_t x){for(unsigned i=0;i<4;i++){p[i]=(uint8_t)x;x>>=8;}}
static void cp(uint8_t *p,const uint8_t *q,unsigned n){while(n--)*p++=*q++;}
static int eq(const uint8_t *p,const uint8_t *q,unsigned n){while(n--)if(*p++!=*q++)return 0;return 1;}
static uint32_t crc(uint32_t c,const uint8_t *p,unsigned n){while(n--){c^=*p++;for(unsigned i=0;i<8;i++)c=(c>>1)^((0u-(c&1u))&0xedb88320u);}return c;}
static unsigned count(uint32_t m){unsigned n=0;while(m){m&=m-1;n++;}return n;}
static unsigned blocks(uint32_t n){return (n+DNP_SECTOR-1)/DNP_SECTOR;}
static unsigned entlen(const uint8_t *e){return ENT+e[16];}
static const uint8_t *find(const directory *d,const uint8_t *id){const uint8_t *e=d->body+2;for(unsigned i=0;i<d->body[1];i++,e+=entlen(e))if(eq(e,id,8))return e;return 0;}
static int layout(const uint8_t *b,unsigned n,uint32_t *used){if(n<2||n>MAXBODY||b[0]>DNP_DATA||b[1]>DNP_DATA)return 0;unsigned at=2;uint32_t mask=(1u<<b[0])-1u;for(unsigned i=0;i<b[1];i++){if(n-at<ENT)return 0;const uint8_t *e=b+at;uint32_t size=le(e+8);unsigned span=e[16];if(size<72||size>65535||span!=blocks(size)||span>n-at-ENT)return 0;const uint8_t *q=b+2;for(unsigned j=0;j<i;j++,q+=entlen(q))if(eq(q,e,8))return 0;for(unsigned j=0;j<span;j++){unsigned s=e[ENT+j];if(s>=DNP_DATA||(mask&(1u<<s)))return 0;mask|=1u<<s;}at+=entlen(e);}if(at!=n)return 0;*used=mask;return 1;}
static int read_dir(const dnpool *p,unsigned slot,directory *d){const uint8_t *h=p->io.flash+slot*DNP_SECTOR;if(le(h)!=MAGIC)return 0;unsigned n=le(h+8);uint32_t g=le(h+4),used;if(!g||n>MAXBODY||n<2||~crc(crc(~0u,h+4,8),h+HDR,n)!=le(h+12)||!layout(h+HDR,n,&used))return -1;d->body=h+HDR;d->gen=g;d->used=used;d->length=n;d->slot=slot;return 1;}
static int mount(const dnpool *p,directory *d){directory a,b;if(!p||!p->io.flash)return DNP_INVALID;int x=read_dir(p,DNP_DATA,&a),y=read_dir(p,DNP_DATA+1,&b);if(x<0||y<0||(!x&&!y)||(x&&y&&a.gen==b.gen))return DNP_CORRUPT;*d=(!y||(x&&a.gen>b.gen))?a:b;return 0;}
static int guard(const dnpool *p){return p&&p->io.flash&&p->io.held&&p->io.erase&&p->io.write&&p->io.held(p->io.ctx);}
static __attribute__((noinline)) int erased(const uint8_t *p,unsigned n){while(n--)if(*p++!=255)return 0;return 1;}
static int erase(dnpool *p,unsigned s){if(s>=DNP_TOTAL||!guard(p)||p->io.erase(p->io.ctx,s*DNP_SECTOR))return DNP_IO;return erased(p->io.flash+s*DNP_SECTOR,DNP_SECTOR)?0:DNP_IO;}
static int write(dnpool *p,unsigned off,const uint8_t *a,unsigned n){if(!n||n>DNP_CHUNK||off>=DNP_BYTES||n>DNP_BYTES-off||!guard(p)||p->io.write(p->io.ctx,off,a,n)||!eq(p->io.flash+off,a,n))return DNP_IO;return 0;}
static void header(uint8_t h[HDR],uint32_t gen,const uint8_t *b,unsigned n){put(h,MAGIC);put(h+4,gen);put(h+8,n);put(h+12,~crc(crc(~0u,h+4,8),b,n));}
static int commit(dnpool *p,unsigned slot,uint32_t gen,const uint8_t *b,unsigned n){uint8_t h[HDR];uint32_t used;if(!gen||!layout(b,n,&used))return DNP_INVALID;header(h,gen,b,n);unsigned off=slot*DNP_SECTOR;/* Retire stale magic before erasing. Never touch active copy. */
    if(le(p->io.flash+off)==MAGIC){uint8_t zero[4]={0,0,0,0};if(write(p,off,zero,4))return DNP_IO;}
    if(erase(p,slot)||write(p,off+4,h+4,12))return DNP_IO;
    for(unsigned j=0;j<n;j+=DNP_CHUNK){unsigned k=n-j;if(k>DNP_CHUNK)k=DNP_CHUNK;if(write(p,off+HDR+j,b+j,k))return DNP_IO;}
    if(~crc(crc(~0u,p->io.flash+off+4,8),p->io.flash+off+HDR,n)!=le(h+12))return DNP_IO;
    if(write(p,off,h,4))return DNP_IO;directory v;return read_dir(p,slot,&v)==1&&v.gen==gen?0:DNP_IO;}
static int edit(dnpool *p,const directory *d,const uint8_t *old,const uint8_t *new_e,unsigned sample){uint8_t b[MAXBODY];unsigned n=2;b[0]=(uint8_t)sample;b[1]=0;const uint8_t *e=d->body+2;for(unsigned i=0;i<d->body[1];i++,e+=entlen(e)){if(e==old)continue;cp(b+n,e,entlen(e));n+=entlen(e);b[1]++;}if(new_e){if(entlen(new_e)>MAXBODY-n)return DNP_FULL;cp(b+n,new_e,entlen(new_e));n+=entlen(new_e);b[1]++;}return commit(p,2*DNP_DATA+1-d->slot,d->gen+1,b,n);}
static uint32_t bankcrc(const dnpool *p,const uint8_t *e){uint32_t c=~0u,n=le(e+8);for(unsigned i=0;i<e[16];i++){unsigned k=n>DNP_SECTOR?DNP_SECTOR:n;c=crc(c,p->io.flash+e[ENT+i]*DNP_SECTOR,k);n-=k;}return ~c;}
static int open_entry(const dnpool *p,const uint8_t *e,dnbank *b){if(!e||!dnbank_open_mapped(b,p->io.flash,DNP_DATA*DNP_SECTOR,e+ENT,e[16],le(e+8))||bankcrc(p,e)!=le(e+12)||!eq(p->io.flash+e[ENT]*DNP_SECTOR+16,e,8))return DNP_CORRUPT;return 0;}
void dnp_init(dnpool *p,const dnp_io *io){p->io=*io;p->active=0;}
int dnp_format(dnpool *p){if(!guard(p)||p->active)return DNP_BUSY;if(!erased(p->io.flash,DNP_BYTES))return DNP_PROTECTED;uint8_t b[2]={0,0};return commit(p,DNP_DATA,1,b,2);}
/* Adopt an existing, validated sample without erasing any sample or bank
 * bytes. Only unused, erased capacity is claimed. The first directory may
 * contain a torn write of exactly the initial directory; all other legacy
 * content (including S1PK or a sample that reaches the directory) is refused. */
int dnp_adopt_sample(dnpool *p){
 if(!guard(p)||p->active)return DNP_BUSY;
 if(!p->io.sample_extent)return DNP_PROTECTED;
 uint32_t end=p->io.sample_extent(p->io.ctx);
 if(!end||end>DNP_DATA*DNP_SECTOR)return DNP_PROTECTED;
 unsigned reserved=blocks(end),start=DNP_DATA*DNP_SECTOR;
 const uint8_t *h=p->io.flash+start;
 if(!erased(p->io.flash+reserved*DNP_SECTOR,start-reserved*DNP_SECTOR)||!erased(h+DNP_SECTOR,DNP_SECTOR))return DNP_PROTECTED;
 directory d;
 if(le(h)==MAGIC){if(read_dir(p,DNP_DATA,&d)==1&&d.gen==1&&d.body[0]==reserved&&!d.body[1])return 0;return DNP_PROTECTED;}
 uint8_t b[2]={(uint8_t)reserved,0},want[HDR+2];header(want,1,b,2);cp(want+HDR,b,2);
 /* Erased bits or any subset of the intended 1->0 transitions are safe
  * to retry; unknown bytes anywhere in the directory sector are not. */
 if(!erased(h+sizeof want,DNP_SECTOR-sizeof want))return DNP_PROTECTED;
 for(unsigned i=0;i<sizeof want;i++)if((h[i]&want[i])!=want[i])return DNP_PROTECTED;
 return commit(p,DNP_DATA,1,b,2);
}
int dnp_get_info(dnpool *p,dnp_info *out){directory d;int rc=mount(p,&d);if(rc||!out)return rc?rc:DNP_INVALID;unsigned s=d.body[0],n=count(d.used);out->generation=d.gen;out->reserved=(uint8_t)s;out->banks=d.body[1];out->bank_sectors=(uint8_t)(n-s);out->free_sectors=(uint8_t)(DNP_DATA-n);while(s<DNP_DATA&&!(d.used&(1u<<s)))s++;out->contiguous=(uint8_t)s;return 0;}
int dnp_open(dnpool *p,const uint8_t id[8],dnbank *b){directory d;int rc=mount(p,&d);if(rc)return rc;if(!id||!b)return DNP_INVALID;const uint8_t *e=find(&d,id);return e?open_entry(p,e,b):DNP_INVALID;}
int dnp_bank_at(dnpool *p,unsigned i,uint8_t id[8],uint32_t *size,uint32_t *c){directory d;int rc=mount(p,&d);if(rc)return rc;if(i>=d.body[1]||!id||!size||!c)return DNP_INVALID;const uint8_t *e=d.body+2;while(i--)e+=entlen(e);cp(id,e,8);*size=le(e+8);*c=le(e+12);return 0;}
int dnp_bank_read(dnpool *p,const uint8_t id[8],uint32_t gen,uint32_t off,uint8_t *out,unsigned n){directory d;int rc=mount(p,&d);if(rc)return rc;if(d.gen!=gen)return DNP_STALE;if(!id||!out||!n||n>DNP_CHUNK)return DNP_INVALID;const uint8_t *e=find(&d,id);if(!e||off>le(e+8)||n>le(e+8)-off)return DNP_INVALID;while(n--){*out++=p->io.flash[e[ENT+off/DNP_SECTOR]*DNP_SECTOR+off%DNP_SECTOR];off++;}return 0;}
static int integrity(dnpool *p,const directory *d){const uint8_t *e=d->body+2;for(unsigned i=0;i<d->body[1];i++,e+=entlen(e))if(bankcrc(p,e)!=le(e+12))return DNP_CORRUPT;return 0;}
int dnp_verify(dnpool *p){directory d;int rc=mount(p,&d);if(rc)return rc;return integrity(p,&d);}
static int writable(dnpool *p,directory *d,uint32_t g){if(!guard(p)||p->active)return DNP_BUSY;int rc=mount(p,d);if(rc)return rc;if(d->gen!=g)return DNP_STALE;if(d->gen==UINT32_MAX)return DNP_FULL;if(!p->io.sample_extent||p->io.sample_extent(p->io.ctx)>d->body[0]*DNP_SECTOR)return DNP_PROTECTED;return integrity(p,d);}
int dnp_begin(dnpool *p,const uint8_t id[8],uint32_t size,uint32_t c,uint32_t gen){directory d;int rc=writable(p,&d,gen);if(rc)return rc;if(!id||size<72||size>65535)return DNP_INVALID;const uint8_t *old=find(&d,id);if(old&&!p->io.allow_replace)return DNP_PROTECTED;unsigned n=blocks(size);if(n>DNP_DATA-count(d.used))return DNP_FULL;unsigned k=0;for(unsigned s=DNP_DATA;s--&&k<n;)if(!(d.used&(1u<<s)))p->map[k++]=(uint8_t)s;cp(p->id,id,8);p->count=(uint8_t)n;p->gen=gen;p->size=size;p->crc=c;p->written=0;p->active=1;return 0;}
int dnp_write(dnpool *p,uint32_t off,const uint8_t *a,unsigned n){if(!guard(p)||!p->active)return DNP_BUSY;directory d;int rc=mount(p,&d);if(rc||d.gen!=p->gen){p->active=0;return rc?rc:DNP_STALE;}if(!a||off>p->size||n!=((p->size-off>DNP_CHUNK)?DNP_CHUNK:p->size-off)||!n||off%DNP_CHUNK)return DNP_INVALID;unsigned dst=p->map[off/DNP_SECTOR]*DNP_SECTOR+off%DNP_SECTOR;if(off<p->written)return off+n<=p->written&&eq(p->io.flash+dst,a,n)?0:DNP_INVALID;if(off!=p->written)return DNP_INVALID;if(!(off%DNP_SECTOR)&&erase(p,p->map[off/DNP_SECTOR])){p->active=0;return DNP_IO;}if(write(p,dst,a,n)){p->active=0;return DNP_IO;}p->written+=n;return 0;}
void dnp_abort(dnpool *p){if(p)p->active=0;}
int dnp_end(dnpool *p){if(!guard(p)||!p->active)return DNP_BUSY;if(p->written!=p->size)return DNP_INVALID;directory d;int rc=mount(p,&d);p->active=0;if(rc||d.gen!=p->gen)return rc?rc:DNP_STALE;uint8_t e[ENT+16];cp(e,p->id,8);put(e+8,p->size);put(e+12,p->crc);e[16]=p->count;cp(e+ENT,p->map,p->count);dnbank b,a;if(open_entry(p,e,&b)||!dnbank_validate(&b))return DNP_CORRUPT;const uint8_t *old=find(&d,p->id);if(old){if(open_entry(p,old,&a))return DNP_CORRUPT;if(!p->io.allow_replace||!p->io.allow_replace(p->io.ctx,&a,&b))return DNP_PROTECTED;}return edit(p,&d,old,e,d.body[0]);}
int dnp_remove(dnpool *p,const uint8_t id[8],uint32_t gen){directory d;int rc=writable(p,&d,gen);if(rc)return rc;if(!id)return DNP_INVALID;const uint8_t *e=find(&d,id);if(!e)return DNP_INVALID;dnbank a;if(open_entry(p,e,&a))return DNP_CORRUPT;if(!p->io.allow_replace||!p->io.allow_replace(p->io.ctx,&a,0))return DNP_PROTECTED;return edit(p,&d,e,0,d.body[0]);}
static __attribute__((noinline)) int move(dnpool *p,const directory *d,const uint8_t *e,unsigned j,unsigned dest){uint8_t buf[DNP_CHUNK],entry[ENT+16];unsigned src=e[ENT+j];if(erase(p,dest))return DNP_IO;for(unsigned o=0;o<DNP_SECTOR;o+=DNP_CHUNK){cp(buf,p->io.flash+src*DNP_SECTOR+o,DNP_CHUNK);if(write(p,dest*DNP_SECTOR+o,buf,DNP_CHUNK))return DNP_IO;}cp(entry,e,entlen(e));entry[ENT+j]=(uint8_t)dest;if(bankcrc(p,entry)!=le(entry+12))return DNP_CORRUPT;return edit(p,d,e,entry,d->body[0]);}
int dnp_reserve_sample(dnpool *p,unsigned n,uint32_t gen){directory d;int rc=writable(p,&d,gen);if(rc)return rc;if(n<d.body[0]||n>DNP_DATA)return DNP_INVALID;if(n>DNP_DATA-count(d.used)+d.body[0])return DNP_FULL;/* Preflight generation wrap for maximum possible moves + final commit. */if(UINT32_MAX-d.gen<DNP_DATA+1)return DNP_FULL;for(;;){const uint8_t *e=d.body+2,*hit=0;unsigned j=0;for(unsigned i=0;i<d.body[1]&&!hit;i++,e+=entlen(e))for(j=0;j<e[16];j++)if(e[ENT+j]<n){hit=e;break;}if(!hit)break;unsigned dest=DNP_DATA;do{--dest;}while(dest>=n&&(d.used&(1u<<dest))&&dest);if(dest<n||(d.used&(1u<<dest)))return DNP_FULL;rc=move(p,&d,hit,j,dest);if(rc)return rc;rc=mount(p,&d);if(rc)return rc;}return n==d.body[0]?0:edit(p,&d,0,0,n);}
int dnp_release_sample(dnpool *p,uint32_t gen){directory d;int rc=writable(p,&d,gen);if(rc)return rc;if(!p->io.sample_extent)return DNP_PROTECTED;uint32_t end=p->io.sample_extent(p->io.ctx);if(end>d.body[0]*DNP_SECTOR)return DNP_PROTECTED;unsigned n=blocks(end);return n==d.body[0]?0:edit(p,&d,0,0,n);}
