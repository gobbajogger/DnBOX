typedef int (*jsink)(float *,float *,unsigned,unsigned);
static jsink target;
__attribute__((visibility("default"))) void bind_j60(jsink f) {target=f;}
int submit_j60(float *l,float *r,unsigned n,unsigned mode) {return target ? target(l,r,n,mode) : 0;}
