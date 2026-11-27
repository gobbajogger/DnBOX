"""First observable stereo divergence, actual native captures vs owner reconstruction."""
from pathlib import Path
p=Path('sj6-newer-level-boundary/check.py')
s=p.read_text().split('print(json.dumps')[0]
s=s.replace(' def db(n,d):',' return rc,real,z\n def db(n,d):')
exec(compile(s,str(p),'exec'))
parts={}
for role in ('main','sub'):
 rec,real,recon=report(role)
 def modes(v):return (v[0]+v[1])*.5,(v[0]-v[1])*.5
 d={}
 for name,i,j in [('attack',16384,18432),('held',24576,37634),('release',38434,38946),('pending',49459,49875),('chord',53971,81434),('final',82534,83046)]:
  rm,rs=modes(real[:,i:j].astype('f8'));cm,cs=modes(recon[:,i:j].astype('f8'))
  norm=np.linalg.norm;db=lambda a,b:float(20*np.log10(max(norm(a),1e-30)/max(norm(b),1e-30)))
  d[name]={'native_side_vs_mid_dB':db(rs,rm),'recon_side_vs_mid_dB':db(cs,cm),
   'mid_level_dB':db(cm,rm),'mid_residual_dBr':db(cm-rm,rm),'side_residual_vs_native_total_dBr':db(cs-rs,real[:,i:j])}
 parts[role]={'pcm_sha':rec['pcm_sha256'],'windows':d}
print(json.dumps(parts,indent=2))
