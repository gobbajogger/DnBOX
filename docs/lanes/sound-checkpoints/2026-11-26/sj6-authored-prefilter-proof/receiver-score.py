from math import degrees,atan2,hypot,log10
def rd(path):
 return {(int(w),int(l),int(k)):(r,p,[(c,s) for c,s in zip(d[::2],d[1::2])]) for w,l,k,r,p,m,*d in (map(float,x.split()) for x in open(path))}
ref={
(1,57):(.261792115,None,.029086782,None,.010470358),
(1,69):(.261788017,None,.029082706,None,.010466306),
(2,57):(.183418608,.091707894,.061137018,.045851113,.036679188,.030564262),
(2,69):(.183415783,.091702225,.061128524,.045839790,.036665044,.030547294),
(3,57):(.228808332,None,.076266305,None,.045756016),
(3,69):(.228804795,None,.076255723,None,.045738387),
}
pre=rd('preclock-3.txt')
post=rd('clock-3.txt')
for (w,l,k),a in pre.items():
 b=post[w,l,k]
 if l==2:
  assert pre[w,1,k]==a and post[w,1,k]==b
  continue
 def phase(x):return degrees(atan2(-x[1],x[0]))
 db=lambda x,y:20*log10(x/y)
 err=[(i+1,round(db(hypot(*a[2][i]),v),6)) for i,v in enumerate(ref[w,k]) if v]
 p0,p1=phase(a[2][0]),phase(b[2][0])
 print(w,k,f'pre {p0:+.9f}, post {p1:+.9f}, d {p1-p0:+.9f}, db {db(hypot(*b[2][0]),hypot(*a[2][0])):+.9f}',err)
