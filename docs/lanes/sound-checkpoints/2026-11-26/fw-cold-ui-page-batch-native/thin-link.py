from pathlib import Path
import os,sys
p,t=map(Path,sys.argv[1:])
root=p/'firmware';dst=t/'tree'/'firmware';base=t/'base'
base.symlink_to(p,target_is_directory=True)
branch={Path(v) for v in ['.','src','src/app','src/dsp']}
def view(src,dest):
 relative=src.relative_to(root)
 if relative not in branch:
  dest.symlink_to(os.path.relpath(base/'firmware'/relative,dest.parent),target_is_directory=src.is_dir())
 else:
  dest.mkdir()
  for child in src.iterdir():view(child,dest/child.name)
view(root,dst)
