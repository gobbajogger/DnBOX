from pathlib import Path
import sys
p,q=map(Path,sys.argv[1:])
s=p.read_text();d=q.read_text().splitlines()
orig,=tuple(x[1:] for x in d if x.startswith('-') and not x.startswith('---'))
sub='\n'.join(x[1:] for x in d if x.startswith('+') and not x.startswith('+++'))
a='defined(X0X_HOST) && X0X_HOST && '
assert s.count(orig)==1 and sub.count(a)==1
p.write_text(s.replace(orig,sub.replace(a,'')))
