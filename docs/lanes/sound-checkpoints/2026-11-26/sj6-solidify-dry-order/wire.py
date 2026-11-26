from pathlib import Path
import sys
g,d=map(Path,sys.argv[1:])
s=(g/'suffix-runtime.c').read_text()
def replace(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
a='''#if (defined(X0X_HOST) && defined(X0X_SJ6_ALT3_WITNESS)) || X0X_SJ6_ALT3_NATIVE
#if X0X_SJ6_AUX_LFO_NATIVE
            if(mod && (mod[3]>=0.f || mod[4]>=0.f))'''
replace(a,'''#if X0X_SJ6_EXACT_ZERO_LEAF
#error This isolated proof rejects duplication with the compact owner.
#endif
    extern void sj6_ordered_hp(float*,float*,unsigned);
    sj6_ordered_hp(left,right,frames);
'''+a)
a='''            juno6_alt3_process(&p->dsp.juno.synth,left,right,frames); /* after HPF, before downstream FX */
#endif'''
replace(a,a+'''
    extern void sj6_ordered_crusher(float*,float*,unsigned);
    sj6_ordered_crusher(left,right,frames);''')
(d/'runtime.c').write_text(s)
s=(g/'suffix-driver.c').read_text()
a='''int sj6_validated_dry_pre(float*l,float*r,unsigned n){
 if(!suffix_scope)return 0;
 suffix_pre+=n; sj6_pre_dry(&routed_suffix,l,r,n);return 1;
}'''
replace(a,'''static unsigned hp_frames,crusher_frames;
void sj6_ordered_hp(float*l,float*r,unsigned n){
 if(!ORIGINAL_DRY_ORDER || !suffix_scope)return;
 assert(hp_frames==crusher_frames && hp_frames==suffix_post);
 hp_frames+=n;
 for(unsigned i=0;i<n;i++){
  l[i]=sj6_hp10_44100(&routed_suffix.pre[0],l[i]);
  r[i]=sj6_hp10_44100(&routed_suffix.pre[1],r[i]);
 }
}
void sj6_ordered_crusher(float*l,float*r,unsigned n){
 if(!ORIGINAL_DRY_ORDER || !suffix_scope)return;
 assert(hp_frames==crusher_frames+n);
 crusher_frames+=n;
 for(unsigned i=0;i<n;i++){
  l[i]=sj6_zero_crusher_44100(&routed_suffix.pre[0],l[i]);
  r[i]=sj6_zero_crusher_44100(&routed_suffix.pre[1],r[i]);
 }
}
int sj6_validated_dry_pre(float*l,float*r,unsigned n){
 if(!suffix_scope)return 0;
 suffix_pre+=n;
 if(!ORIGINAL_DRY_ORDER)sj6_pre_dry(&routed_suffix,l,r,n);
 return 1; /* admitted physical Crusher already ran in ordered path */
}''')
a='  assert(!fclose(pre));'
replace(a,'  fprintf(stderr,"before_trem_HPF=%u post_trem_Crusher=%u post_master_DC=%u\\n",hp_frames,crusher_frames,suffix_post);\n'+a)
(d/'driver.c').write_text(s)
