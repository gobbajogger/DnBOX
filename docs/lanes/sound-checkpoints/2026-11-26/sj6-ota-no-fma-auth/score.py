import sys
from pathlib import Path
s=Path(sys.argv[1]).read_text()
s=s.replace("for i,a in enumerate(('base','fused','separate')) if i","for i,a in enumerate(('fused_52b','packed_64b','muladd_52b')) if i")
s=s.replace("[(1,'fused'),(2,'separate')]","[(1,'packed_64b'),(2,'muladd_52b')]")
a="   rows[f'{start_name}:{k}']="
assert s.count(a)==1
s=s.replace(a,"   H=np.exp(-2j*np.pi*np.outer(np.arange(1,9),np.arange(i,j))*440*2**((k-69)/12)/44100)\n   phase=lambda x,y:np.angle((H@x)/(H@y),deg=True).tolist()\n"+a)
s=s.replace('original=stats(z[i],z[3]))','original=stats(z[i],z[3]),H1_to_H8_phase_vs_fused=phase(z[i],z[0]),H1_to_H8_phase_vs_original=phase(z[i],z[3]))')
s=s.replace("r=n(a-b)", "r=n(a-b)")
Path(sys.argv[2]).write_text(s)
