// Checked own-adapter operations. Original void/assert methods stay untouched.
#include "scheduler.hpp"
#include "float_math.h"
namespace s1scheduler {
int init_morph_checked(sp_data *sp,sp_oscmorph2d *osc,sp_ftbl **ft,int nft,int nbl,float *fbls,SPFLOAT iphs) {
    // This OWN initializer has the actual engine's fixed52x4096/zero-phase
    // domain; the original general Soundpipe function remains for the oracle.
    if(!sp||!osc||!ft||!fbls||nft!=4||nbl!=13||iphs!=0.f)return SP_NOT_OK;
    for(unsigned i=0;i<52;++i)if(!ft[i]||ft[i]->size!=4096)return SP_NOT_OK;
    for(unsigned i=0;i<13;++i)if(!s1_finitef(fbls[i])||(i&&fbls[i-1]>fbls[i]))return SP_NOT_OK;
    // Exact original successful initialization at its source call iphs=0.
    osc->freq=440.0f;osc->amp=.2f;osc->tbl=ft;osc->iphs=0.f;osc->inc=0;osc->lphs=0;
    osc->wtpos=0.f;osc->nft=nft;osc->nbl=nbl;osc->fbl=fbls;
    osc->enableBandlimit=0;osc->bandlimitIndexOverride=-1;return SP_OK;
}

bool Held::remove_checked(size_t i) {
    if(!valid_count() || i>=static_cast<size_t>(count))return false;
    for(size_t n=i+1;n<static_cast<size_t>(count);++n)values[n-1]=values[n];
    --count;return true;
}
bool Held::insert_checked(NoteNumber value,size_t i) {
    if(!valid_count() || count>=128 || i>static_cast<size_t>(count))return false;
    for(size_t n=static_cast<size_t>(count);n>i;--n)values[n]=values[n-1];
    values[i]=value;++count;return true;
}
bool EmptySequencer::process_checked(const float *p,const Held &held)const {
    return p && p[arpIsOn]==0.f && pending_empty && held.valid_count();
}
}
