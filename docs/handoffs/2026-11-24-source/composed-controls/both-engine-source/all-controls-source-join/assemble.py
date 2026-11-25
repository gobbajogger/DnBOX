import pathlib,re,subprocess,sys
root=pathlib.Path(sys.argv[1]); order=pathlib.Path(sys.argv[2])
for path in map(pathlib.Path,order.read_text().splitlines()):
 s=path.read_bytes()
 parts=re.split(rb'(?m)(?=^--- [ab]/|^--- /dev/null)',s)
 for x in parts:
  m=re.search(rb'(?m)^\+\+\+ (?:b/)?(\S+)',x)
  if not m:continue
  file=m[1].decode()
  # Source recovery for UI/editor is incomplete; keep their hypothetical
  # context-match results separate from exact DSP handoff.
  opts=['patch','-p1','--batch','--forward','--fuzz=0','--no-backup-if-mismatch']
  r=subprocess.run([*opts,'--dry-run'],input=x,cwd=root,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
  if r.returncode:
   print('CONFLICT',path.name,file,r.stdout.decode().strip().replace('\n',' | '),flush=True)
   continue
  r=subprocess.run(opts,input=x,cwd=root,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,check=True)
  print('APPLIED',path.name,file,r.stdout.decode().strip().replace('\n',' | '),flush=True)
