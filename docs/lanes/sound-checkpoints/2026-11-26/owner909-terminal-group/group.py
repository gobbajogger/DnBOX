import sys,hashlib
from pathlib import Path
p=Path(sys.argv[1]); s=p.read_text()
assert hashlib.sha256(p.read_bytes()).hexdigest()=='69e6f18b56ef6c710ed018dd352d5c79eac5faabc1fe653b40606287c8a6c5d7'
def replace(a,b,n=1):
 global s
 assert s.count(a)==n,(a,s.count(a))
 s=s.replace(a,b)
a='static void d9_render(drum909_t *d, float *dry, float *rev, float *dly, int n, uint16_t audible_tracks)'
replace(a,'''/* Quarantined fixture only. Actual firmware may not opt in. */
#include <stdatomic.h>
typedef struct {d9_smp_t voices[4];float mono[4][256];_Atomic unsigned ready;} terminal_future;
static unsigned terminal_token(unsigned stamp,unsigned n)
{
    return ((stamp&0x7fffffu)<<9)|((n-1u)<<1);
}
static void terminal_begin(terminal_future*f,const drum909_t*d,unsigned token)
{
    for(unsigned i=0;i<4;i++)f->voices[i]=d->smp[i];
    atomic_store_explicit(&f->ready,token,memory_order_relaxed);
}
static void terminal_create(terminal_future*f,int n,unsigned token)
{
    for(unsigned i=0;i<4;i++){
        d9_bus_t b={.dry=0,.y=f->mono[i]};
        if(f->voices[i].mute>0&&f->voices[i].playing)
            d9_render_smp(&f->voices[i],&b,n);
    }
    atomic_store_explicit(&f->ready,token|1u,memory_order_release);
}
'''+a[:-1]+',terminal_future *future,unsigned expected)')
replace('''    static const uint8_t order[4] = { DR_OH, DR_RD, DR_CR, DR_CH };''','''    static const uint8_t order[4] = { DR_OH, DR_RD, DR_CR, DR_CH };
    const int prepared = future && atomic_load_explicit(&future->ready,memory_order_acquire)==(expected|1u);''')
replace('''            d9_render_smp(s, &bus, n);''','''            if(prepared && s->playing) {
                int count=s->mute<n?s->mute:n;
                unsigned i=(unsigned)order[k]-DR_CH;
                *s=future->voices[i];
                bus.y=future->mono[i];
                d9_emit_block(&bus,count);
                bus.y=y;
            } else d9_render_smp(s, &bus, n);''')
replace('d9_render(d, dry, rev, dly, n, 0x7FFu);','d9_render(d, dry, rev, dly, n, 0x7FFu,0,0);')
replace('int n, uint16_t audible_tracks, int cleared)','int n, uint16_t audible_tracks, int cleared,terminal_future *future,unsigned expected)')
replace('d9_render(d, dry_l, rev, dly, n, audible_tracks);','d9_render(d, dry_l, rev, dly, n, audible_tracks,future,expected);')
replace('audible_tracks,1);','audible_tracks,1,0,0);')
replace('audible_tracks,0);','audible_tracks,0,0,0);')
print(s)
